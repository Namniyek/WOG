// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGDefender.h"
#include "WOG.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "WOG/ActorComponents/WOGBuildComponent.h"
#include "AbilitySystemComponent.h"
#include "Types/WOGGameplayTags.h"
#include "Components/AGR_EquipmentManager.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Magic/WOGBaseMagic.h"

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
		if (Magic && HasMatchingGameplayTag(Magic->GetMagicData().CooldownTag))
		{
			UE_LOG(LogTemp, Error, TEXT("Cooldown in effect. Can't equip"));
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
	if (Magic && HasMatchingGameplayTag(Magic->GetMagicData().CooldownTag))
	{
		UE_LOG(LogTemp, Error, TEXT("Cooldown in effect. Can't equip"));
		return;
	}

	//Execute ability
	SendAbilityLocalInput(EWOGAbilityInputID::Ability3);
}
