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
	if (HasMatchingGameplayTag(TAG_State_Dead)) return;
	if (HasMatchingGameplayTag(TAG_State_Dodging)) return;
	if (HasMatchingGameplayTag(TAG_State_Debuff_Knockback)) return;
	if (HasMatchingGameplayTag(TAG_State_Debuff_KO)) return;
	if (HasMatchingGameplayTag(TAG_State_Debuff_Stagger)) return;

	FVector2D AbilitiesVector = Value.Get<FVector2D>();

	if (AbilitiesVector.X > 0)
	{
		//Button 4/Right pressed


	}
	if (AbilitiesVector.X < 0)
	{
		//Button 1/Left pressed
		if (!EquipmentManager)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString("Equipment component invalid"));
			return;
		}
		AActor* OutItem = nullptr;
		AActor* PrimaryItem = nullptr;
		EquipmentManager->GetWeaponShortcutReference(FName("1"), OutItem);
		EquipmentManager->GetItemInSlot(NAME_WeaponSlot_Primary, PrimaryItem);
		if (PrimaryItem && OutItem && PrimaryItem == OutItem)
		{
			FGameplayEventData EventPayload;
			EventPayload.EventTag = TAG_Event_Weapon_Unequip;
			EventPayload.OptionalObject = PrimaryItem;
			int32 Key = FCString::Atoi(*FName("1").ToString());
			EventPayload.EventMagnitude = Key;
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, TAG_Event_Weapon_Unequip, EventPayload);
		}
		else if (OutItem)
		{
			FGameplayEventData EventPayload;
			EventPayload.EventTag = TAG_Event_Weapon_Equip;
			EventPayload.OptionalObject = OutItem;
			int32 Key = FCString::Atoi(*FName("1").ToString());
			EventPayload.EventMagnitude = Key;
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, TAG_Event_Weapon_Equip, EventPayload);
		}
	}
	if (AbilitiesVector.Y > 0)
	{
		//Button 2/Up pressed


	}
	if (AbilitiesVector.Y < 0)
	{
		//Button 3/Down pressed

	}
}
