// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGDefender.h"
#include "WOG.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "WOG/ActorComponents/WOGBuildComponent.h"
#include "AbilitySystemComponent.h"
#include "Data/WOGGameplayTags.h"
#include "Components/AGR_EquipmentManager.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Magic/WOGBaseMagic.h"
#include "AbilitySystem/AttributeSets/WOGAttributeSetBase.h"
#include "Subsystems/WOGUIManagerSubsystem.h"
#include "WOG/PlayerController/WOGPlayerController.h"
#include "Net/UnrealNetwork.h"

AWOGDefender::AWOGDefender()
{
	BuildComponent = CreateDefaultSubobject<UWOGBuildComponent>(TEXT("BuildingComponent"));
	BuildComponent->SetIsReplicated(true);

	CurrentMeleeSquad = nullptr;
	CurrentRangedSquad = nullptr;
}

void AWOGDefender::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWOGDefender, CurrentMeleeSquad);
	DOREPLIFETIME(AWOGDefender, CurrentRangedSquad);
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

		//Ability
		EnhancedInputComponent->BindAction(Ability3HoldAction, ETriggerEvent::Started, this, &ThisClass::Ability3HoldButtonStarted);
		EnhancedInputComponent->BindAction(Ability3HoldAction, ETriggerEvent::Ongoing, this, TEXT("AbilityHoldButtonElapsed"));
		EnhancedInputComponent->BindAction(Ability3HoldAction, ETriggerEvent::Canceled, this, &ThisClass::AbilityHoldButtonCanceled);
		EnhancedInputComponent->BindAction(Ability3HoldAction, ETriggerEvent::Triggered, this, &ThisClass::Ability3HoldButtonTriggered);
	}
}

void AWOGDefender::InteractActionPressed(const FInputActionValue& Value)
{
	//TO-DO interact functionality
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
		SendAbilityLocalInput(EWOGAbilityInputID::Ability2);
	}
	if (AbilitiesVector.Y < 0)
	{
		//Button 3/Down pressed

		if (!EquipmentManager) return;

		AActor* OutMagic = nullptr;
		EquipmentManager->GetMagicShortcutReference(FName("1"), OutMagic);
		TObjectPtr<AWOGBaseMagic> Magic = Cast<AWOGBaseMagic>(OutMagic);
		if (Magic && Magic->GetMagicData().AbilityInputType != EAbilityInputType::EAI_Instant) return;

		//Check for Cooldown
		if (Magic && HasMatchingGameplayTag(Magic->GetMagicData().CooldownTag))
		{
			UE_LOG(WOGLogCombat, Error, TEXT("Cooldown in effect. Can't equip"));
			return;
		}

		//Check for resource
		bool bSucessCheck = false;
		if (AttributeSet && Magic && Magic->GetMagicData().Cost > UAbilitySystemBlueprintLibrary::GetFloatAttribute(this, AttributeSet->GetAdrenalineAttribute(), bSucessCheck))
		{
			TObjectPtr<UWOGUIManagerSubsystem> UIManager = ULocalPlayer::GetSubsystem<UWOGUIManagerSubsystem>(OwnerPC->GetLocalPlayer());
			if (UIManager)
			{
				UIManager->CreateResourceWarningWidget(FString("Adrenaline"));
				UE_LOG(WOGLogCombat, Error, TEXT("Not enough Adrenaline. Can't equip"));
			}
			return;
		}

		SendAbilityLocalInput(EWOGAbilityInputID::Ability3);
	}
}

void AWOGDefender::Ability3HoldButtonStarted(const FInputActionValue& Value)
{
	AbilityHoldStarted(FName("1"));
}

void AWOGDefender::Ability3HoldButtonTriggered(const FInputActionValue& Value)
{
	//Button 3/Down pressed

	//Remove hold bar widget
	RemoveHoldProgressBarWidget();

	if (!EquipmentManager) return;

	AActor* OutMagic = nullptr;
	EquipmentManager->GetMagicShortcutReference(FName("1"), OutMagic);
	TObjectPtr<AWOGBaseMagic> Magic = Cast<AWOGBaseMagic>(OutMagic);
	if (Magic && Magic->GetMagicData().AbilityInputType != EAbilityInputType::EAI_Hold) return;

	//Check for Cooldown
	if (Magic && HasMatchingGameplayTag(Magic->GetMagicData().CooldownTag))
	{
		UE_LOG(WOGLogCombat, Error, TEXT("Cooldown in effect. Can't equip"));
		return;
	}

	//Check for resource
	bool bSucessCheck = false;
	if (AttributeSet && Magic && Magic->GetMagicData().Cost > UAbilitySystemBlueprintLibrary::GetFloatAttribute(this, AttributeSet->GetAdrenalineAttribute(), bSucessCheck))
	{
		TObjectPtr<UWOGUIManagerSubsystem> UIManager = ULocalPlayer::GetSubsystem<UWOGUIManagerSubsystem>(OwnerPC->GetLocalPlayer());
		if (UIManager)
		{
			UIManager->CreateResourceWarningWidget(FString("Adrenaline"));
			UE_LOG(WOGLogCombat, Error, TEXT("Not enough Adrenaline. Can't equip"));
		}
		return;
	}

	//Execute ability
	SendAbilityLocalInput(EWOGAbilityInputID::Ability3);
}

bool AWOGDefender::IsCurrentMeleeSquadSlotAvailable_Implementation() const
{
	return CurrentMeleeSquad == nullptr;
}

bool AWOGDefender::IsCurrentRangedSquadSlotAvailable_Implementation() const
{
	return CurrentRangedSquad == nullptr;
}

void AWOGDefender::FreeCurrentRangedSquadSlot_Implementation()
{
	SetCurrentRangedSquad(nullptr);
}

void AWOGDefender::FreeCurrentMeleeSquadSlot_Implementation()
{
	SetCurrentMeleeSquad(nullptr);
}

void AWOGDefender::SetCurrentRangedSquadSlot_Implementation(AWOGBaseSquad* NewSquad)
{
	SetCurrentRangedSquad(NewSquad);
}

void AWOGDefender::SetCurrentMeleeSquadSlot_Implementation(AWOGBaseSquad* NewSquad)
{
	SetCurrentMeleeSquad(NewSquad);
}

bool AWOGDefender::ReserveAttackTokens_Implementation(const int32& AmountToReserve)
{
	if (!HasAuthority()) return false;

	if (AvailableAttackTokens < AmountToReserve)
	{
		return false;
	}

	AvailableAttackTokens = FMath::Clamp(AvailableAttackTokens - AmountToReserve, 0, MaxAttackTokens);
	return true;
}

void AWOGDefender::RestoreAttackTokens_Implementation(const int32& AmountToRestore)
{
	if (!HasAuthority())
	{
		UE_LOG(WOGLogCombat, Error, TEXT("No authority when attempting to restore attack tokens"));
		return;
	}

	AvailableAttackTokens = FMath::Clamp(AvailableAttackTokens + AmountToRestore, 0, MaxAttackTokens);
	UE_LOG(WOGLogCombat, Display, TEXT("Tokens restored: %d, now AvailableTokens: %d"), AmountToRestore, AvailableAttackTokens);
}

int32 AWOGDefender::GetAvailableAttackTokens_Implementation()
{
	return AvailableAttackTokens;
}

void AWOGDefender::SetCurrentRangedSquad(AWOGBaseSquad* NewSquad)
{
	if (HasAuthority())
	{
		CurrentRangedSquad = NewSquad;
	}
}

void AWOGDefender::SetCurrentMeleeSquad(AWOGBaseSquad* NewSquad)
{
	if (HasAuthority())
	{
		CurrentMeleeSquad = NewSquad;
	}
}
