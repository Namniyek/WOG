// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGBaseCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "PlayerState/WOGPlayerState.h"
#include "ActorComponents/WOGAbilitySystemComponent.h"
#include "GameplayEffectExtension.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/AttributeSets/WOGAttributeSetBase.h"
#include "AbilitySystem/Abilities/WOGGameplayAbilityBase.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "PlayerCharacter/BasePlayerCharacter.h"
#include "Enemies/WOGBaseEnemy.h"
#include "Types/WOGGameplayTags.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MotionWarpingComponent.h"
#include "Components/CapsuleComponent.h"
#include "Blueprint/UserWidget.h"
#include "PlayerController/WOGPlayerController.h"
#include "Components/WidgetComponent.h"
#include "Components/SizeBox.h"
#include "UI/WOGCharacterWidgetContainer.h"

AWOGBaseCharacter::AWOGBaseCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	AbilitySystemComponent = CreateDefaultSubobject<UWOGAbilitySystemComponent>(TEXT("Ability System Component"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->ReplicationMode = EGameplayEffectReplicationMode::Full;

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetHealthAttribute()).AddUObject(this, &ThisClass::OnHealthAttributeChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetStaminaAttribute()).AddUObject(this, &ThisClass::OnStaminaAttributeChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetMaxMovementSpeedAttribute()).AddUObject(this, &ThisClass::OnMaxMovementSpeedAttributeChanged);
	AbilitySystemComponent->OnGameplayEffectAppliedDelegateToSelf.AddUObject(this, &ThisClass::OnGameplayEffectAppliedToSelf);

	AttributeSet = CreateDefaultSubobject<UWOGAttributeSetBase>(TEXT("AttributeSet"));

	CombatManager = CreateDefaultSubobject<UAGR_CombatManager>(TEXT("CombatManager"));
	CombatManager->SetIsReplicated(true);
	CombatManager->OnStartAttack.AddDynamic(this, &ThisClass::OnStartAttack);
	CombatManager->OnAttackHitEvent.AddDynamic(this, &ThisClass::OnAttackHit);

	AnimManager = CreateDefaultSubobject<UAGRAnimMasterComponent>(TEXT("AnimManager"));
	AnimManager->SetIsReplicated(true);

	MotionWarping = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarping"));

	StaminaWidgetContainer = CreateDefaultSubobject<UWidgetComponent>(TEXT("Stamina Widget Container"));
	StaminaWidgetContainer->SetWidgetSpace(EWidgetSpace::Screen);
	StaminaWidgetContainer->SetDrawAtDesiredSize(true);
	StaminaWidgetContainer->SetupAttachment(GetRootComponent());

	SpeedRequiredForLeap = 750.f;

	LastHitResult = FHitResult();

	bIsRagdolling = false;
	bIsLayingOnBack = false;
	MeshLocation = FVector();
	TargetGroundLocation = FVector();
	PelvisOffset = FVector();
	SpringState = FVectorSpringState();
}

void AWOGBaseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWOGBaseCharacter, bIsRagdolling);
}

void AWOGBaseCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}

	// ASC MixedMode replication requires that the ASC Owner's Owner be the Controller.
	SetOwner(NewController);
}

void AWOGBaseCharacter::BeginPlay()
{
	Super::BeginPlay();

	InitPhysics();
}

void AWOGBaseCharacter::SendAbilityLocalInput(const EWOGAbilityInputID InInputID)
{
	if (!AbilitySystemComponent.Get())
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid AbilitySystemComponent"));
		return;
	}

	AbilitySystemComponent->AbilityLocalInputPressed(static_cast<int32>(InInputID));
}

void AWOGBaseCharacter::GiveDefaultAbilities()
{
	if (!HasAuthority() || DefaultAbilitiesAndEffects.Abilities.IsEmpty() || !AbilitySystemComponent.Get()) return;
	for (auto DefaultAbility : DefaultAbilitiesAndEffects.Abilities)
	{
		AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(DefaultAbility, 1, static_cast<int32>(DefaultAbility.GetDefaultObject()->AbilityInputID), this));
	}
}

void AWOGBaseCharacter::ApplyDefaultEffects()
{
	if (!HasAuthority() || DefaultAbilitiesAndEffects.Effects.IsEmpty() || !AbilitySystemComponent.Get()) return;

	FGameplayEffectContextHandle EffectContext = AbilitySystemComponent.Get()->MakeEffectContext();
	EffectContext.AddSourceObject(this);

	for (auto DefaultEffect : DefaultAbilitiesAndEffects.Effects)
	{
		
		ApplyGameplayEffectToSelf(DefaultEffect, EffectContext);
	}
}

bool AWOGBaseCharacter::ApplyGameplayEffectToSelf(TSubclassOf<UGameplayEffect> Effect, FGameplayEffectContextHandle InEffectContext, float Duration)
{
	if (!Effect.Get()) return false;

	FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(Effect, Duration, InEffectContext);
	if (SpecHandle.IsValid())
	{
		FActiveGameplayEffectHandle ActiveGEHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

		return ActiveGEHandle.WasSuccessfullyApplied();
	}
	return false;
}

void AWOGBaseCharacter::OnHealthAttributeChanged(const FOnAttributeChangeData& Data)
{
	if (Data.NewValue <= 0 && Data.OldValue > 0 && HasAuthority())
	{
		if (Data.GEModData)
		{
			const FGameplayEffectContextHandle& EffectContext = Data.GEModData->EffectSpec.GetContext();

			if (EffectContext.GetInstigator()->IsA<ABasePlayerCharacter>())
			{
				ACharacter* InstigatorCharacter = Cast<ACharacter>(EffectContext.GetInstigator());
				if (InstigatorCharacter && InstigatorCharacter->GetController())
				{
					FGameplayEventData EventPayload;
					EventPayload.EventTag = TAG_Event_Elim;
					EventPayload.Instigator = InstigatorCharacter->GetController();
					UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, TAG_Event_Elim, EventPayload);
					UE_LOG(LogTemp, Error, TEXT("Killed by Character"));
				}
			}
			else if (EffectContext.GetInstigator()->IsA<AWOGBaseEnemy>())
			{
				FGameplayEventData EventPayload;
				EventPayload.EventTag = TAG_Event_Elim;
				UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, TAG_Event_Elim, EventPayload);
				UE_LOG(LogTemp, Error, TEXT("Killed by Enemy"));

				/*
				** TO-DO - Add and pass reference to the Owner of the enemy that killed this character
				*/
			}
		}
	}
}

void AWOGBaseCharacter::OnStaminaAttributeChanged(const FOnAttributeChangeData& Data)
{
	if (Data.NewValue < Data.OldValue && !UKismetMathLibrary::NearlyEqual_FloatFloat(Data.NewValue, AttributeSet->GetMaxStamina(), 1.f))
	{
		TObjectPtr<AWOGPlayerController> OwnerPC = Cast<AWOGPlayerController>(Controller);
		if (OwnerPC && OwnerPC->IsLocalController())
		{
			OwnerPC->AddStaminaWidget();
		}
	}
}

void AWOGBaseCharacter::OnMaxMovementSpeedAttributeChanged(const FOnAttributeChangeData& Data)
{
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = Data.NewValue;
	}
}

void AWOGBaseCharacter::OnGameplayEffectAppliedToSelf(UAbilitySystemComponent* Source, const FGameplayEffectSpec& Spec, FActiveGameplayEffectHandle Handle)
{
	if (!HasAuthority()) return;
	FGameplayTagContainer GrantedTags;
	Spec.GetAllGrantedTags(GrantedTags);
	for (auto Tag : GrantedTags)
	{
		if (Tag == TAG_State_Debuff_Freeze)
		{
			Server_SetCharacterFrozen(true);
			UE_LOG(LogTemp, Display, TEXT("Server_SetCharacterFrozen(true) called"));
		}
	}
}

void AWOGBaseCharacter::OnStartAttack()
{
	HitActorsToIgnore.Empty();
}

void AWOGBaseCharacter::OnAttackHit(FHitResult Hit, UPrimitiveComponent* WeaponMesh)
{
	//Handle early returns
	if (!Hit.bBlockingHit || !Hit.GetActor()) return;
	if (HitActorsToIgnore.Contains(Hit.GetActor())) return;

	HitActorsToIgnore.AddUnique(Hit.GetActor());
	
	ProcessHit(Hit, WeaponMesh);
}

void AWOGBaseCharacter::Server_SetCharacterFrozen_Implementation(bool bIsFrozen)
{
	Multicast_SetCharacterFrozen(bIsFrozen);
}

void AWOGBaseCharacter::Multicast_SetCharacterFrozen_Implementation(bool bIsFrozen)
{
	SetCharacterFrozen(bIsFrozen);
}

void AWOGBaseCharacter::SetCharacterFrozen_Implementation(bool bIsFrozen)
{
	if (bIsFrozen)
	{
		//Character is freezing
		//Pause animations and stop movemement
		if (GetMesh())
		{
			GetMesh()->bPauseAnims = true;
		}
		if (GetCharacterMovement())
		{
			GetCharacterMovement()->StopMovementImmediately();
		}
	}
	else
	{
		//Character is unfreezing
		TObjectPtr<ABasePlayerCharacter> PlayerCharacter = Cast<ABasePlayerCharacter>(this);
		if (PlayerCharacter && PlayerCharacter->GetMesh())
		{
			int32 MatNum = PlayerCharacter->GetMesh()->GetSkeletalMeshAsset()->GetNumMaterials();
			for (int32 i = 0; i < MatNum; i++)
			{
				PlayerCharacter->GetMesh()->SetMaterial(i, PlayerCharacter->CharacterMI);
			}

			PlayerCharacter->SetColors(
				PlayerCharacter->PlayerProfile.PrimaryColor, 
				PlayerCharacter->PlayerProfile.SkinColor, 
				PlayerCharacter->PlayerProfile.BodyPaintColor, 
				PlayerCharacter->PlayerProfile.HairColor);
		}

		//UnPause animations
		if (GetMesh())
		{
			GetMesh()->bPauseAnims = false;
		}
	}
}

void AWOGBaseCharacter::ToggleStrafeMovement(bool bIsStrafe)
{
	if (!AnimManager) return;
	if (bIsStrafe)
	{
		//Start strafe movement
		AnimManager->SetupAimOffset(EAGR_AimOffsets::Aim, EAGR_AimOffsetClamp::Nearest, 90, false);
		AnimManager->SetupRotation(EAGR_RotationMethod::DesiredAtAngle, 500.f, 90.f, 5.f);

	}
	else
	{
		//Stop strafe movement
		AnimManager->SetupAimOffset(EAGR_AimOffsets::Look, EAGR_AimOffsetClamp::Left, 90, true);
		AnimManager->SetupRotation(EAGR_RotationMethod::RotateToVelocity, 500.f, 90.f, 5.f);
	}
}

bool AWOGBaseCharacter::IsHitFrontal(const float& AngleTolerance, const AActor* Victim, const FVector& Location, const AActor* Agressor)
{
	//We check first if the Agressor actor is valid.
	//If it is we use it, if it's not, we use the vector param.
	FRotator LookAtRotation = Agressor != nullptr ? 
		UKismetMathLibrary::FindLookAtRotation(Victim->GetActorLocation(), Agressor->GetActorLocation()) :
		UKismetMathLibrary::FindLookAtRotation(Victim->GetActorLocation(), Location);

	FRotator DeltaRotator = UKismetMathLibrary::NormalizedDeltaRotator(GetActorRotation(), LookAtRotation);

	bool bIsHitFrontal = 
		!(UKismetMathLibrary::InRange_FloatFloat(DeltaRotator.Yaw, -180, -AngleTolerance) ||
		UKismetMathLibrary::InRange_FloatFloat(DeltaRotator.Yaw, AngleTolerance, 180));

	return bIsHitFrontal;
}

FName AWOGBaseCharacter::CalculateHitDirection(const FVector& WeaponLocation)
{
	// Get the location of the impact point
	FVector ImpactPoint = LastHitResult.Location;
	LastHitDirection = LastHitResult.Normal;
	FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(ImpactPoint, WeaponLocation);
	FRotator DeltaRotator = UKismetMathLibrary::NormalizedDeltaRotator(GetActorRotation(), LookAtRotation);

	if (UKismetMathLibrary::InRange_FloatFloat(DeltaRotator.Yaw, -45, -0) || UKismetMathLibrary::InRange_FloatFloat(DeltaRotator.Yaw, 0, 45))
	{
		//Hit came from the front
		UE_LOG(LogTemp, Warning, TEXT("FRONT"));
		return FName("Front");
	}

	else if (UKismetMathLibrary::InRange_FloatFloat(DeltaRotator.Yaw, -135, -46))
	{
		//Hit came from the right
		UE_LOG(LogTemp, Warning, TEXT("RIGHT"));
		return FName("Right");
	}

	else if (UKismetMathLibrary::InRange_FloatFloat(DeltaRotator.Yaw, 46, 135))
	{
		//Hit came from the left
		UE_LOG(LogTemp, Warning, TEXT("LEFT"));
		return FName("Left");
	}

	else if (UKismetMathLibrary::InRange_FloatFloat(DeltaRotator.Yaw, -180, -136) || UKismetMathLibrary::InRange_FloatFloat(DeltaRotator.Yaw, 136, 180))
	{
		//Hit came from the back
		UE_LOG(LogTemp, Warning, TEXT("BACK"));
		return FName("Back");
	}

	return FName("");
}

void AWOGBaseCharacter::Server_ToRagdoll_Implementation(const float& Radius, const float& Strength, const FVector_NetQuantize& Origin)
{
	bIsRagdolling = true;
	Multicast_ToRagdoll(Radius, Strength, Origin);

	FTimerHandle TimerHandle;

	GetWorldTimerManager().SetTimer(TimerHandle, this, &ThisClass::Server_ToAnimation, 3.f);
}

void AWOGBaseCharacter::Multicast_ToRagdoll_Implementation(const float& Radius, const float& Strength, const FVector_NetQuantize& Origin)
{
	GetMesh()->SetCollisionProfileName(FName("Ragdoll"));
	GetMesh()->SetAllBodiesBelowSimulatePhysics(FName("Pelvis"), true, true);
	GetCharacterMovement()->DisableMovement();
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->bUpdateJointsFromAnimation = true;
	GetMesh()->AddRadialImpulse(Origin, Radius, Strength, ERadialImpulseFalloff::RIF_Constant, true);
}

void AWOGBaseCharacter::Server_ToAnimation_Implementation()
{
	Multicast_ToAnimation();
}

void AWOGBaseCharacter::Multicast_ToAnimation_Implementation()
{
	if (!bIsRagdolling) return;

	FindCharacterOrientation();
	SetCapsuleOrientation();

	FLatentActionInfo LatentActionInfo;
	LatentActionInfo.CallbackTarget = this;
	LatentActionInfo.ExecutionFunction = "ToAnimationSecondStage";
	LatentActionInfo.UUID = 125;
	LatentActionInfo.Linkage = 0;

	UKismetSystemLibrary::Delay(this, 0.01f, LatentActionInfo);
}

void AWOGBaseCharacter::ToAnimationSecondStage()
{
	GetMesh()->GetAnimInstance()->SavePoseSnapshot(FName("RagdollFinalPose"));

	FLatentActionInfo LatentActionInfo;
	LatentActionInfo.CallbackTarget = this;
	LatentActionInfo.ExecutionFunction = "ToAnimationThirdStage";
	LatentActionInfo.UUID = 124;
	LatentActionInfo.Linkage = 0;

	UKismetSystemLibrary::DelayUntilNextTick(this, LatentActionInfo);
}

void AWOGBaseCharacter::ToAnimationThirdStage()
{
	if (HasAuthority())
	{
		bIsRagdolling = false;
	}

	GetMesh()->SetCollisionProfileName(FName("Custom"));
	GetMesh()->SetCollisionResponseToAllChannels(ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECC_GameTraceChannel6, ECR_Block);
	GetMesh()->SetAllBodiesSimulatePhysics(false);
	GetMesh()->bUpdateJointsFromAnimation = false;

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

void AWOGBaseCharacter::ToAnimationFinal()
{
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking, 0);
}

void AWOGBaseCharacter::FindCharacterOrientation()
{
	FRotator Rotation = GetMesh()->GetSocketRotation(FName("Pelvis"));
	FVector RotationRightVector = UKismetMathLibrary::GetRightVector(Rotation);

	bIsLayingOnBack = RotationRightVector.Z < 0.f;
}

void AWOGBaseCharacter::SetCapsuleOrientation()
{
	FVector NeckLocation = GetMesh()->GetSocketLocation(FName("neck_01"));
	FVector PelvisLocation = GetMesh()->GetSocketLocation(FName("Pelvis"));

	FVector Orientation = bIsLayingOnBack ? ((NeckLocation-PelvisLocation) * -1) : (NeckLocation - PelvisLocation);

	FRotator NewRotation = UKismetMathLibrary::MakeRotFromXZ(Orientation, FVector::UpVector);
}

void AWOGBaseCharacter::UpdateCapsuleLocation()
{
	if (bIsRagdolling)
	{
		FHitResult Hit;
		FVector Start = GetMesh()->GetSocketLocation(FName("Pelvis"));
		FVector End = Start - FVector(0, 0, 100);
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(this);

		GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, QueryParams);

		if (Hit.bBlockingHit)
		{
			TargetGroundLocation = Hit.Location;
		}
		else
		{
			TargetGroundLocation = GetMesh()->GetSocketLocation(FName("Pelvis"));
		}

		MeshLocation = UKismetMathLibrary::VectorSpringInterp(
			MeshLocation,
			(TargetGroundLocation - PelvisOffset),
			SpringState,
			400.f,
			1.f,
			UGameplayStatics::GetWorldDeltaSeconds(this));

		GetCapsuleComponent()->SetWorldLocation(MeshLocation);
		
	}
	else
	{
		MeshLocation = GetCapsuleComponent()->GetComponentLocation();
		
		FVector PhysicsLinearVelocity = GetMesh()->GetPhysicsLinearVelocity();
		SpringState.Velocity = PhysicsLinearVelocity;
	}
}

void AWOGBaseCharacter::InitPhysics()
{
	PelvisOffset = GetMesh()->GetRelativeLocation();
}

void AWOGBaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateCapsuleLocation();
}

UWOGCharacterWidgetContainer* AWOGBaseCharacter::GetStaminaWidgetContainer() const
{
	if (!StaminaWidgetContainer) return nullptr;
	TObjectPtr<UWOGCharacterWidgetContainer> StaminaContainer = Cast<UWOGCharacterWidgetContainer>(StaminaWidgetContainer->GetWidget());
	return StaminaContainer;
}

void AWOGBaseCharacter::AddHoldProgressBar()
{
	TObjectPtr<AWOGPlayerController> OwnerController = Cast<AWOGPlayerController>(Controller);
	if (!OwnerController || !IsLocallyControlled()) return;

	OwnerController->AddHoldProgressBar();
}

void AWOGBaseCharacter::RemoveHoldProgressBarWidget()
{
	TObjectPtr<AWOGPlayerController> OwnerController = Cast<AWOGPlayerController>(Controller);
	if (!OwnerController || !IsLocallyControlled()) return;

	OwnerController->RemoveHoldProgressBar();
}

void AWOGBaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

bool AWOGBaseCharacter::HasMatchingGameplayTag(FGameplayTag TagToCheck) const
{
	if (GetAbilitySystemComponent())
	{
		return GetAbilitySystemComponent()->HasMatchingGameplayTag(TagToCheck);
	}

	return false;
}

UAbilitySystemComponent* AWOGBaseCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent.Get();
}

