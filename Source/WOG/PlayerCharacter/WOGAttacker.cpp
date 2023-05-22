// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGAttacker.h"
#include "TargetingHelperComponent.h"
#include "LockOnTargetComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "WOG/PlayerController/WOGPlayerController.h"
#include "WOG/ActorComponents/WOGSpawnComponent.h"
#include "WOG/ActorComponents/WOGCombatComponent.h"
#include "AbilitySystemComponent.h"
#include "Types/WOGGameplayTags.h"


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
	LockOnTarget->EnableTargeting();
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
	PossessMinion();
}

void AWOGAttacker::RotateSpawnActionPressed(const FInputActionValue& Value)
{
	if (HasMatchingGameplayTag(TAG_State_Dead)) return;
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
	if (!SpawnComponent) return;
	SpawnComponent->PlaceSpawn();
}

void AWOGAttacker::AbilitiesButtonPressed(const FInputActionValue& Value)
{
	if (HasMatchingGameplayTag(TAG_State_Dead)) return;
	if (HasMatchingGameplayTag(TAG_State_Dodging)) return;
	if (CharacterState == ECharacterState::ECS_Staggered) return;

	FVector2D AbilitiesVector = Value.Get<FVector2D>();

	if (AbilitiesVector.X > 0)
	{
		//Button 4/Right pressed


	}
	if (AbilitiesVector.X < 0)
	{
		//Button 1/Left pressed
		if (!Combat)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString("Combat component invalid"));
			return;
		}

		if (!Combat->EquippedWeapon)
		{
			Combat->EquipMainWeapon();
		}
		else if (Combat->EquippedWeapon)
		{
			Combat->UnequipMainWeapon();
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
