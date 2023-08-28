// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGAttacker.h"
#include "WOG.h"
#include "TargetSystemComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "WOG/PlayerController/WOGPlayerController.h"
#include "WOG/ActorComponents/WOGSpawnComponent.h"
#include "AbilitySystemComponent.h"
#include "Types/WOGGameplayTags.h"
#include "Components/AGR_EquipmentManager.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "ActorComponents/WOGAbilitySystemComponent.h"
#include "Magic/WOGBaseMagic.h"


AWOGAttacker::AWOGAttacker()
{
	SpawnComponent = CreateDefaultSubobject<UWOGSpawnComponent>(TEXT("SpawnComponent"));
	SpawnComponent->SetIsReplicated(true);
}

void AWOGAttacker::BeginPlay()
{
	Super::BeginPlay();
}

void AWOGAttacker::PossessMinion()
{
	OwnerPC = OwnerPC == nullptr ? Cast<AWOGPlayerController>(GetController()) : OwnerPC;
	if (!OwnerPC)
	{
		UE_LOG(LogTemp, Error, TEXT("invalid OwnerPC"));
		return;
	}

	if (!CurrentTarget)
	{
		UE_LOG(LogTemp, Error, TEXT("invalid CurrentTarget"));
		return;
	}

	OwnerPC->PossessMinion(CurrentTarget);
	TargetComponent->TargetLockOff();
}

void AWOGAttacker::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		//Possess
		EnhancedInputComponent->BindAction(PossessAction, ETriggerEvent::Completed, this, &ThisClass::PossessActionPressed);
		//Spawn 
		EnhancedInputComponent->BindAction(SpawnAction, ETriggerEvent::Completed, this, &ThisClass::SpawnActionPressed);
		EnhancedInputComponent->BindAction(RotateSpawnAction, ETriggerEvent::Triggered, this, &ThisClass::RotateSpawnActionPressed);
	}
}

void AWOGAttacker::PossessActionPressed(const FInputActionValue& Value)
{
	if (HasMatchingGameplayTag(TAG_State_Dead)) return;
	if (HasMatchingGameplayTag(TAG_State_Debuff_Knockback)) return;
	if (HasMatchingGameplayTag(TAG_State_Debuff_KO)) return;
	if (HasMatchingGameplayTag(TAG_State_Debuff_Stagger)) return;

	PossessMinion();
}

void AWOGAttacker::RotateSpawnActionPressed(const FInputActionValue& Value)
{
	if (HasMatchingGameplayTag(TAG_State_Dead)) return;
	if (HasMatchingGameplayTag(TAG_State_Debuff_Knockback)) return;
	if (HasMatchingGameplayTag(TAG_State_Debuff_KO)) return;
	if (HasMatchingGameplayTag(TAG_State_Debuff_Stagger)) return;
	if (!SpawnComponent) return;

	float Direction = Value.Get<float>();

	if (Direction < 0)
	{
		//Rotate left
		SpawnComponent->HandleSpawnRotation(true);
	}
	else
	{
		//Rotate right
		SpawnComponent->HandleSpawnRotation(false);
	}
}

void AWOGAttacker::SpawnActionPressed(const FInputActionValue& Value)
{
	if (HasMatchingGameplayTag(TAG_State_Dead)) return;
	if (HasMatchingGameplayTag(TAG_State_Debuff_Knockback)) return;
	if (HasMatchingGameplayTag(TAG_State_Debuff_KO)) return;
	if (HasMatchingGameplayTag(TAG_State_Debuff_Stagger)) return;
	if (!SpawnComponent) return;

	SpawnComponent->PlaceSpawn();
}

void AWOGAttacker::AbilitiesButtonPressed(const FInputActionValue& Value)
{
	FVector2D AbilitiesVector = Value.Get<FVector2D>();

	if (AbilitiesVector.X > 0)
	{
		//Button 4/Right pressed
		SendAbilityLocalInput(EWOGAbilityInputID::Ability4);
	}
	if (AbilitiesVector.X < 0)
	{
		//Button 1/Left pressed
		SendAbilityLocalInput(EWOGAbilityInputID::Ability1);
	}
	if (AbilitiesVector.Y > 0)
	{
		//Button 2/Up pressed

		if (!EquipmentManager) return;

		//Check for cooldown tag and 
		AActor* OutMagic = nullptr;
		EquipmentManager->GetMagicShortcutReference(FName("1"), OutMagic);
		if (!OutMagic) return;

		TObjectPtr<AWOGBaseMagic> MagicToEquip = Cast<AWOGBaseMagic>(OutMagic);
		if (MagicToEquip && MagicToEquip->GetMagicData().AbilityInputType != EAbilityInputType::EAI_Instant) return;
		if (MagicToEquip && HasMatchingGameplayTag(MagicToEquip->GetMagicData().CooldownTag))
		{
			UE_LOG(LogTemp, Error, TEXT("Cooldown in effect. Can't equip"));
			return;
		}

		SendAbilityLocalInput(EWOGAbilityInputID::Ability2);
	}
	if (AbilitiesVector.Y < 0)
	{
		//Button 3/Down pressed
		if (!EquipmentManager) return;

		//Check for cooldown tag
		AActor* OutMagic = nullptr;
		EquipmentManager->GetMagicShortcutReference(FName("2"), OutMagic);
		if (!OutMagic) return;

		TObjectPtr<AWOGBaseMagic> MagicToEquip = Cast<AWOGBaseMagic>(OutMagic);
		if (MagicToEquip && MagicToEquip->GetMagicData().AbilityInputType != EAbilityInputType::EAI_Instant) return;
		if (MagicToEquip && HasMatchingGameplayTag(MagicToEquip->GetMagicData().CooldownTag))
		{
			UE_LOG(LogTemp, Error, TEXT("Cooldown in effect. Can't equip"));
			return;
		}

		SendAbilityLocalInput(EWOGAbilityInputID::Ability3);
	}
}

void AWOGAttacker::AbilitiesHoldButtonPressed(const FInputActionValue& Value)
{
	FVector2D AbilitiesVector = Value.Get<FVector2D>();

	if (AbilitiesVector.Y > 0)
	{
		//Button 2/Up pressed
		if (!EquipmentManager) return;

		//Check for cooldown tag
		AActor* OutMagic = nullptr;
		EquipmentManager->GetMagicShortcutReference(FName("1"), OutMagic);
		if (!OutMagic) return;

		TObjectPtr<AWOGBaseMagic> MagicToEquip = Cast<AWOGBaseMagic>(OutMagic);
		if (MagicToEquip && MagicToEquip->GetMagicData().AbilityInputType != EAbilityInputType::EAI_Hold) return;
		if (MagicToEquip && HasMatchingGameplayTag(MagicToEquip->GetMagicData().CooldownTag))
		{
			UE_LOG(LogTemp, Error, TEXT("Cooldown in effect. Can't equip"));
			return;
		}

		//Execute ability
		SendAbilityLocalInput(EWOGAbilityInputID::Ability2);
	}
	if (AbilitiesVector.Y < 0)
	{
		//Button 3/Down pressed
		if (!EquipmentManager) return;

		//Check for cooldown tag
		AActor* OutMagic = nullptr;
		EquipmentManager->GetMagicShortcutReference(FName("2"), OutMagic);
		if (!OutMagic) return;

		TObjectPtr<AWOGBaseMagic> MagicToEquip = Cast<AWOGBaseMagic>(OutMagic);
		if (MagicToEquip && MagicToEquip->GetMagicData().AbilityInputType != EAbilityInputType::EAI_Hold) return;
		if (MagicToEquip && HasMatchingGameplayTag(MagicToEquip->GetMagicData().CooldownTag))
		{
			UE_LOG(LogTemp, Error, TEXT("Cooldown in effect. Can't equip"));
			return;
		}

		//Execute ability
		SendAbilityLocalInput(EWOGAbilityInputID::Ability3);
	}
}
