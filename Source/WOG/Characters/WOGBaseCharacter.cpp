// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGBaseCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
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


AWOGBaseCharacter::AWOGBaseCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	AbilitySystemComponent = CreateDefaultSubobject<UWOGAbilitySystemComponent>(TEXT("Ability System Component"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->ReplicationMode = EGameplayEffectReplicationMode::Mixed;

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetHealthAttribute()).AddUObject(this, &ThisClass::OnHealthAttributeChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetMaxMovementSpeedAttribute()).AddUObject(this, &ThisClass::OnMaxMovementSpeedAttributeChanged);

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
	}

	// ASC MixedMode replication requires that the ASC Owner's Owner be the Controller.
	SetOwner(NewController);
}


void AWOGBaseCharacter::BeginPlay()
{
	Super::BeginPlay();
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

void AWOGBaseCharacter::OnMaxMovementSpeedAttributeChanged(const FOnAttributeChangeData& Data)
{
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = Data.NewValue;
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
	case ECharacterState::ECS_Attacking:
		HandleStateAttacking();
		break;
	case ECharacterState::ECS_Staggered:
		HandleStateStaggered();
		break;
	}
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

void AWOGBaseCharacter::Elim(bool bPlayerLeftGame)
{
	//To be overriden in Children
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

