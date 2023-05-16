// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGDefender.h"
#include "TargetingHelperComponent.h"
#include "LockOnTargetComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "WOG/ActorComponents/WOGBuildComponent.h"
#include "WOG/ActorComponents/WOGCombatComponent.h"
#include "WOG/ActorComponents/WOGAbilitiesComponent.h"
#include "AbilitySystemComponent.h"

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
	if (!GetAbilitySystemComponent() || GetAbilitySystemComponent()->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(TEXT("State.Dead")))) return;
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
	if (!GetAbilitySystemComponent() || GetAbilitySystemComponent()->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(TEXT("State.Dead")))) return;
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
	if (!GetAbilitySystemComponent() || GetAbilitySystemComponent()->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(TEXT("State.Dead")))) return;
	if (!BuildComponent) return;
	BuildComponent->PlaceBuildable();
}

void AWOGDefender::AbilitiesButtonPressed(const FInputActionValue& Value)
{
	if (!GetAbilitySystemComponent() || GetAbilitySystemComponent()->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(TEXT("State.Dead")))) return;
	if (CharacterState == ECharacterState::ECS_Staggered) return;
	if (CharacterState == ECharacterState::ECS_Dodging) return;

	FVector2D AbilitiesVector = Value.Get<FVector2D>();

	if (AbilitiesVector.X > 0)
	{
		//Button 4/Right pressed
		if (!Combat)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString("Combat component invalid"));
			return;
		}
		if (Combat->EquippedWeapon)
		{
			Combat->StoreEquippedWeapon();
		}

		if (!Abilities)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString("Abilities component invalid"));
			return;
		}

		Abilities->Server_EquipAbility(1);

	}
	if (AbilitiesVector.X < 0)
	{
		//Button 1/Left pressed
		if (!Abilities)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString("Abilities component invalid"));
			return;
		}
		if (Abilities->EquippedAbility)
		{
			Abilities->Server_UnequipAbility();
		}

		if (!Combat)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString("Combat component invalid"));
			return;
		}
		if (!Combat->EquippedWeapon)
		{
			Combat->EquipMainWeapon();
		}
		else if (Combat->EquippedWeapon == Combat->SecondaryWeapon)
		{
			Combat->SwapWeapons();
		}
		else if (Combat->EquippedWeapon == Combat->MainWeapon)
		{
			Combat->UnequipMainWeapon();
		}
	}
	if (AbilitiesVector.Y > 0)
	{
		//Button 2/Up pressed
		if (!Abilities)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString("Abilities component invalid"));
			return;
		}
		if (Abilities->EquippedAbility)
		{
			Abilities->Server_UnequipAbility();
		}

		if (!Combat)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString("Combat component invalid"));
			return;
		}
		if (!Combat->SecondaryWeapon)
		{
			Combat->Server_CreateSecondaryWeapon(Combat->SecondaryWeaponClass);
		}
		else if (!Combat->EquippedWeapon)
		{
			Combat->EquipSecondaryWeapon();
		}
		else if (Combat->EquippedWeapon == Combat->MainWeapon)
		{
			Combat->SwapWeapons();
		}
		else if (Combat->EquippedWeapon == Combat->SecondaryWeapon)
		{
			Combat->UnequipSecondaryWeapon();
		}

	}
	if (AbilitiesVector.Y < 0)
	{
		//Button 3/Down pressed
		if (!Combat)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString("Combat component invalid"));
			return;
		}
		if (Combat->EquippedWeapon)
		{
			Combat->StoreEquippedWeapon();
		}

		if (!Abilities)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString("Abilities component invalid"));
			return;
		}

		Abilities->Server_EquipAbility(0);
	}
}
