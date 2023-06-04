// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGDefender.h"
#include "WOG.h"
#include "TargetingHelperComponent.h"
#include "LockOnTargetComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "WOG/ActorComponents/WOGBuildComponent.h"
#include "AbilitySystemComponent.h"
#include "Types/WOGGameplayTags.h"
#include "Components/AGR_EquipmentManager.h"
#include "AbilitySystemBlueprintLibrary.h"

AWOGDefender::AWOGDefender()
{
	BuildComponent = CreateDefaultSubobject<UWOGBuildComponent>(TEXT("BuildingComponent"));
	BuildComponent->SetIsReplicated(true);
}

void AWOGDefender::DestroyComponent(UActorComponent* ComponentToDestroy)
{
	if (ComponentToDestroy)
	{
		ComponentToDestroy->DestroyComponent();
	}
}

void AWOGDefender::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		//Interact
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Triggered, this, &ThisClass::InteractActionPressed);

		//Spawn 
		EnhancedInputComponent->BindAction(SpawnAction, ETriggerEvent::Completed, this, &ThisClass::SpawnActionPressed);
		EnhancedInputComponent->BindAction(RotateSpawnAction, ETriggerEvent::Triggered, this, &ThisClass::RotateSpawnActionPressed);
		EnhancedInputComponent->BindAction(AdjustSpawnHeightAction, ETriggerEvent::Triggered, this, &ThisClass::AdjustSpawnHeightActionPressed);
	}
}

void AWOGDefender::InteractActionPressed(const FInputActionValue& Value)
{
	//TO-DO interact functionality
	UE_LOG(LogTemp, Warning, TEXT("Interact button pressed from C++"));
}

void AWOGDefender::AdjustSpawnHeightActionPressed(const FInputActionValue& Value)
{
	if (HasMatchingGameplayTag(TAG_State_Dead)) return;
	if (HasMatchingGameplayTag(TAG_State_Debuff_Knockback)) return;
	if (HasMatchingGameplayTag(TAG_State_Debuff_KO)) return;
	if (HasMatchingGameplayTag(TAG_State_Debuff_Stagger)) return;
	if (!BuildComponent) return;

	float Direction = Value.Get<float>();

	if (Direction > 0)
	{
		//Raise Build
		BuildComponent->HandleBuildHeight(true);
	}
	else
	{
		//Lower right
		BuildComponent->HandleBuildHeight(false);
	}
}

void AWOGDefender::RotateSpawnActionPressed(const FInputActionValue& Value)
{
	if (HasMatchingGameplayTag(TAG_State_Dead)) return;
	if (HasMatchingGameplayTag(TAG_State_Debuff_Knockback)) return;
	if (HasMatchingGameplayTag(TAG_State_Debuff_KO)) return;
	if (HasMatchingGameplayTag(TAG_State_Debuff_Stagger)) return;

	if (!BuildComponent) return;
	float Direction = Value.Get<float>();

	if (Direction < 0)
	{
		//Rotate left
		BuildComponent->HandleBuildRotation(true);
	}
	else
	{
		//Rotate right
		BuildComponent->HandleBuildRotation(false);
	}
}

void AWOGDefender::SpawnActionPressed(const FInputActionValue& Value)
{
	if (HasMatchingGameplayTag(TAG_State_Dead)) return;
	if (HasMatchingGameplayTag(TAG_State_Debuff_Knockback)) return;
	if (HasMatchingGameplayTag(TAG_State_Debuff_KO)) return;
	if (HasMatchingGameplayTag(TAG_State_Debuff_Stagger)) return;
	if (!BuildComponent) return;

	BuildComponent->PlaceBuildable();
}

void AWOGDefender::AbilitiesButtonPressed(const FInputActionValue& Value)
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


		if (!EquipmentManager)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString("Equipment component invalid"));
			return;
		}
		AActor* OutItem = nullptr;
		AActor* PrimaryItem = nullptr;
		EquipmentManager->GetWeaponShortcutReference(FName("2"), OutItem);
		EquipmentManager->GetItemInSlot(NAME_WeaponSlot_Primary, PrimaryItem);
		if (PrimaryItem && OutItem && PrimaryItem == OutItem)
		{
			FGameplayEventData EventPayload;
			EventPayload.EventTag = TAG_Event_Weapon_Unequip;
			EventPayload.OptionalObject = PrimaryItem;
			int32 Key = FCString::Atoi(*FName("2").ToString());
			EventPayload.EventMagnitude = Key;
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, TAG_Event_Weapon_Unequip, EventPayload);
		}
		else if (OutItem)
		{
			FGameplayEventData EventPayload;
			EventPayload.EventTag = TAG_Event_Weapon_Equip;
			EventPayload.OptionalObject = OutItem;
			int32 Key = FCString::Atoi(*FName("2").ToString());
			EventPayload.EventMagnitude = Key;
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, TAG_Event_Weapon_Equip, EventPayload);
		}
	}
	if (AbilitiesVector.Y < 0)
	{
		//Button 3/Down pressed

	}
}
