// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGBaseCharacter.h"
#include "Net/UnrealNetwork.h"
#include "ActorComponents/WOGAttributesComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerState/WOGPlayerState.h"
#include "ActorComponents/WOGAbilitySystemComponent.h"
#include "GameplayEffectExtension.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/AttributeSets/WOGAttributeSetBase.h"
#include "AbilitySystem/Abilities/WOGGameplayAbilityBase.h"


AWOGBaseCharacter::AWOGBaseCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	Attributes = CreateDefaultSubobject<UWOGAttributesComponent>(TEXT("AttributesComponent"));
	Attributes->SetIsReplicated(true);

	AbilitySystemComponent = CreateDefaultSubobject<UWOGAbilitySystemComponent>(TEXT("Ability System Component"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->ReplicationMode = EGameplayEffectReplicationMode::Mixed;

	AttributeSet = CreateDefaultSubobject<UWOGAttributeSetBase>(TEXT("AttributeSet"));

	SpeedRequiredForLeap = 750.f;
}

void AWOGBaseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWOGBaseCharacter, CharacterState);
}

void AWOGBaseCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetHealthAttribute()).AddUObject(this, &ThisClass::OnHealthAttributeChanged);

	}

	// ASC MixedMode replication requires that the ASC Owner's Owner be the Controller.
	SetOwner(NewController);
}


void AWOGBaseCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &ThisClass::ReceiveDamage);
	}
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

bool AWOGBaseCharacter::ApplyGameplayEffectToSelf(TSubclassOf<UGameplayEffect> Effect, FGameplayEffectContextHandle InEffectContext)
{
	if (!Effect.Get()) return false;

	FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(Effect, 1, InEffectContext);
	if (SpecHandle.IsValid())
	{
		FActiveGameplayEffectHandle ActiveGEHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

		return ActiveGEHandle.WasSuccessfullyApplied();
	}
	return false;
}

void AWOGBaseCharacter::OnHealthAttributeChanged(const FOnAttributeChangeData& Data)
{
	if (Data.NewValue <= 0 && Data.OldValue > 0)
	{
		AWOGBaseCharacter* InstigatorCharacter = nullptr;
		if (Data.GEModData)
		{
			const FGameplayEffectContextHandle& EffectContext = Data.GEModData->EffectSpec.GetContext();
			InstigatorCharacter = Cast<AWOGBaseCharacter>(EffectContext.GetInstigator());

			if (InstigatorCharacter && InstigatorCharacter->GetController())
			{
				Server_SetCharacterState(ECharacterState::ECS_Elimmed, InstigatorCharacter->GetController());
			}
		}

		/*FGameplayEventData EventPayload;
		EventPayload.EventTag = ZeroHealthEventTag;

		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, ZeroHealthEventTag, EventPayload);*/
	}
}

void AWOGBaseCharacter::Server_SetCharacterState_Implementation(ECharacterState NewState, AController* InstigatedBy)
{
	Multicast_SetCharacterState(NewState, InstigatedBy);
	SetCharacterState(NewState, InstigatedBy);
}

void AWOGBaseCharacter::Multicast_SetCharacterState_Implementation(ECharacterState NewState, AController* InstigatedBy)
{
	if (!HasAuthority())
	{
		SetCharacterState(NewState, InstigatedBy);
	}
}

void AWOGBaseCharacter::SetCharacterState(ECharacterState NewState, AController* InstigatedBy)
{
	CharacterState = NewState;

	switch (CharacterState)
	{
	case ECharacterState::ECS_Elimmed:
		HandleStateElimmed(InstigatedBy);
		break;
	case ECharacterState::ECS_Sprinting:
		HandleStateSprinting();
		break;
	case ECharacterState::ECS_Dodging :
		HandleStateDodging();
		break;
	case ECharacterState::ECS_Unnoccupied:
		HandleStateUnnoccupied();
		break;
	case ECharacterState::ECS_Attacking:
		HandleStateAttacking();
		break;
	case ECharacterState::ECS_Staggered:
		HandleStateStaggered();
		break;
	}
}

void AWOGBaseCharacter::HandleStateElimmed(AController* InstigatedBy)
{
	//To be overriden in Children
}

//void AWOGBaseCharacter::HandleStateSprinting()
//{
//	//To be overriden in Children
//}

void AWOGBaseCharacter::HandleStateUnnoccupied()
{
	//To be overriden in Children
}

void AWOGBaseCharacter::HandleStateDodging()
{
	//To be overriden in Children
}

void AWOGBaseCharacter::HandleStateAttacking()
{
	//To be overriden in Children
}

void AWOGBaseCharacter::HandleStateStaggered()
{
	//To be overriden in Children
}

void AWOGBaseCharacter::BroadcastHit_Implementation(AActor* AgressorActor, const FHitResult& Hit, const float& DamageToApply, AActor* InstigatorWeapon)
{

}

bool AWOGBaseCharacter::IsHitFrontal(const float& AngleTolerance, const AActor* Victim, const AActor* Agressor)
{
	FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(Victim->GetActorLocation(), Agressor->GetActorLocation());
	FRotator DeltaRotator = UKismetMathLibrary::NormalizedDeltaRotator(GetActorRotation(), LookAtRotation);

	bool bIsHitFrontal = 
		!(UKismetMathLibrary::InRange_FloatFloat(DeltaRotator.Yaw, -180, -AngleTolerance) ||
		UKismetMathLibrary::InRange_FloatFloat(DeltaRotator.Yaw, AngleTolerance, 180));

	return bIsHitFrontal;
}

void AWOGBaseCharacter::Multicast_HandleCosmeticHit_Implementation(const ECosmeticHit& HitType, const FHitResult& Hit, const FVector& WeaponLocation, const AWOGBaseWeapon* InstigatorWeapon)
{
	switch (HitType)
	{
	case ECosmeticHit::ECH_BodyHit:
		HandleCosmeticBodyHit(Hit, WeaponLocation, InstigatorWeapon);
		break;
	case ECosmeticHit::ECH_BlockingWeapon:
		HandleCosmeticBlock(InstigatorWeapon);
		break;
	case ECosmeticHit::ECH_AttackingWeapon:
		HandleCosmeticWeaponClash();
		break;
	}
}

void AWOGBaseCharacter::HandleCosmeticBodyHit(const FHitResult& Hit, const FVector& WeaponLocation, const AWOGBaseWeapon* InstigatorWeapon)
{
	//To be overriden in Children
}

FName AWOGBaseCharacter::CalculateHitDirection(const FHitResult& Hit, const FVector& WeaponLocation)
{
	// Get the location of the impact point
	FVector ImpactPoint = Hit.Location;

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

	//// Calculate the attack direction
	//FVector AttackDirection = (ImpactPoint - WeaponLocation).GetSafeNormal();

	//// Calculate the hit direction
	//FVector CharacterForward = GetActorForwardVector();
	//LastHitDirection = FVector::CrossProduct(AttackDirection, CharacterForward);

	//// Determine the hit direction
	//if (FMath::Abs(LastHitDirection.Z) > FMath::Abs(LastHitDirection.X) && FMath::Abs(LastHitDirection.Z) > FMath::Abs(LastHitDirection.Y))
	//{
	//	// Hit came from the front or back
	//	if (FVector::DotProduct(AttackDirection, CharacterForward) > 0.0f)
	//	{
	//		// Hit came from the front
	//		UE_LOG(LogTemp, Warning, TEXT("FRONT"));
	//		return FName("Front");
	//	}
	//	else
	//	{
	//		// Hit came from the back
	//		UE_LOG(LogTemp, Warning, TEXT("BACK"));
	//		return FName("Back");
	//	}
	//}
	//else
	//{
	//	// Hit came from the left or right
	//	if (LastHitDirection.Z < 0.0f)
	//	{
	//		// Hit came from the right
	//		UE_LOG(LogTemp, Warning, TEXT("RIGHT"));
	//		return FName("Right");
	//	}
	//	else
	//	{
	//		// Hit came from the left
	//		UE_LOG(LogTemp, Warning, TEXT("LEFT"));
	//		return FName("Left");
	//	}
	//}
}

void AWOGBaseCharacter::PlayHitReactMontage(FName Section)
{
	//To be overriden in Children
}

void AWOGBaseCharacter::HandleCosmeticBlock(const AWOGBaseWeapon* InstigatorWeapon)
{
	//To be overriden in Children
}

void AWOGBaseCharacter::HandleCosmeticWeaponClash()
{
	//To be overriden in Children
}

void AWOGBaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWOGBaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AWOGBaseCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if (!Attributes) return;
	Attributes->Server_UpdateHealth(Damage, InstigatedBy);
}

void AWOGBaseCharacter::Elim(bool bPlayerLeftGame)
{
	//To be overriden in Children
}

UAbilitySystemComponent* AWOGBaseCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent.Get();
}

