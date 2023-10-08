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
#include "Kismet/GameplayStatics.h"
#include "Enemies/WOGRaven.h"
#include "Net/UnrealNetwork.h"


AWOGAttacker::AWOGAttacker()
{
	SpawnComponent = CreateDefaultSubobject<UWOGSpawnComponent>(TEXT("SpawnComponent"));
	SpawnComponent->SetIsReplicated(true);
}

void AWOGAttacker::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWOGAttacker, Raven);
}

void AWOGAttacker::BeginPlay()
{
	Super::BeginPlay();

	SetAllocatedRaven();
}

void AWOGAttacker::SetAllocatedRaven()
{
	TArray<AActor*> OutActors;
	FName Tag = FName(*FString::FromInt(PlayerProfile.UserIndex));
	UGameplayStatics::GetAllActorsOfClassWithTag(this, AWOGRaven::StaticClass(), Tag, OutActors);

	if (OutActors.IsEmpty()) return;

	Raven = Cast<AWOGRaven>(OutActors[0]);
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

		//Abilities
		EnhancedInputComponent->BindAction(Ability2HoldAction, ETriggerEvent::Started, this, &ThisClass::Ability2HoldButtonStarted);
		EnhancedInputComponent->BindAction(Ability2HoldAction, ETriggerEvent::Ongoing, this, TEXT("AbilityHoldButtonElapsed"));
		EnhancedInputComponent->BindAction(Ability2HoldAction, ETriggerEvent::Canceled, this, &ThisClass::AbilityHoldButtonCanceled);
		EnhancedInputComponent->BindAction(Ability2HoldAction, ETriggerEvent::Triggered, this, &ThisClass::Ability2HoldButtonTriggered);

		EnhancedInputComponent->BindAction(Ability3HoldAction, ETriggerEvent::Started, this, &ThisClass::Ability3HoldButtonStarted);
		EnhancedInputComponent->BindAction(Ability3HoldAction, ETriggerEvent::Ongoing, this, TEXT("AbilityHoldButtonElapsed"));
		EnhancedInputComponent->BindAction(Ability3HoldAction, ETriggerEvent::Canceled, this, &ThisClass::AbilityHoldButtonCanceled);
		EnhancedInputComponent->BindAction(Ability3HoldAction, ETriggerEvent::Triggered, this, &ThisClass::Ability3HoldButtonTriggered);
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

void AWOGAttacker::Ability2HoldButtonStarted(const FInputActionValue& Value)
{
	AbilityHoldStarted(FName("1"));
}

void AWOGAttacker::Ability2HoldButtonTriggered(const FInputActionValue& Value)
{
	//Button 2/Up pressed

	//Remove hold bar widget
	RemoveHoldProgressBarWidget();

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

void AWOGAttacker::Ability3HoldButtonStarted(const FInputActionValue& Value)
{
	AbilityHoldStarted(FName("2"));
}

void AWOGAttacker::Ability3HoldButtonTriggered(const FInputActionValue& Value)
{
	//Button 3/Down pressed

	//Remove hold bar widget
	RemoveHoldProgressBarWidget();

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

void AWOGAttacker::PossessRaven()
{
	if (!Raven)
	{
		UE_LOG(LogTemp, Error, TEXT("No valid raven reference"));
		return;
	}

	OwnerPC = OwnerPC == nullptr ? Cast<AWOGPlayerController>(GetController()) : OwnerPC;
	if (!OwnerPC)
	{
		UE_LOG(LogTemp, Error, TEXT("invalid OwnerPC"));
		return;
	}

	Raven->SetActorRotation(FRotator());
	Raven->bUseControllerRotationPitch = true;
	Raven->bUseControllerRotationYaw = true;
	UE_LOG(LogTemp, Warning, TEXT("Raven is player controlled from AttackerCharacter: %s"), *UEnum::GetValueAsString(GetLocalRole()));

	OwnerPC->PossessMinion(Raven);
	TargetComponent->TargetLockOff();
}
