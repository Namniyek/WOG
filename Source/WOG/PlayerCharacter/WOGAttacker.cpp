// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGAttacker.h"
#include "WOG.h"
#include "TargetSystemComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "WOG/PlayerController/WOGPlayerController.h"
#include "WOG/ActorComponents/WOGSpawnComponent.h"
#include "AbilitySystemComponent.h"
#include "Data/WOGGameplayTags.h"
#include "Components/AGR_EquipmentManager.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "ActorComponents/WOGAbilitySystemComponent.h"
#include "Magic/WOGBaseMagic.h"
#include "Kismet/GameplayStatics.h"
#include "Enemies/WOGRaven.h"
#include "Enemies/WOGMinerGiant.h"
#include "Net/UnrealNetwork.h"
#include "AbilitySystem/AttributeSets/WOGAttributeSetBase.h"
#include "Kismet/KismetMathLibrary.h"
#include "Interfaces/SpawnInterface.h"
#include "Subsystems/WOGUIManagerSubsystem.h"
#include "Libraries/WOGBlueprintLibrary.h"
#include "ActorComponents/WOGEnemyOrderComponent.h"
#include "Camera/CameraComponent.h"


AWOGAttacker::AWOGAttacker()
{
	SpawnComponent = CreateDefaultSubobject<UWOGSpawnComponent>(TEXT("SpawnComponent"));
	SpawnComponent->SetIsReplicated(true);

	CurrentExternalMinion = nullptr;
	bCanPossessMinion = true;

	SquadSlot_0 = CreateDefaultSubobject<USceneComponent>(TEXT("SquadSlot_0"));
	SquadSlot_0->SetupAttachment(GetRootComponent());
	SquadSlot_1 = CreateDefaultSubobject<USceneComponent>(TEXT("SquadSlot_1"));
	SquadSlot_1->SetupAttachment(GetRootComponent());
	SquadSlot_2 = CreateDefaultSubobject<USceneComponent>(TEXT("SquadSlot_2"));
	SquadSlot_2->SetupAttachment(GetRootComponent());

	EnemyOrderComponent = CreateDefaultSubobject<UWOGEnemyOrderComponent>(TEXT("Enemy Order Component"));
	EnemyOrderComponent->SetIsReplicated(true);
}

void AWOGAttacker::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWOGAttacker, Raven);
	DOREPLIFETIME(AWOGAttacker, Miner);
	DOREPLIFETIME(AWOGAttacker, CurrentExternalMinion);
	DOREPLIFETIME(AWOGAttacker, bCanPossessMinion);
}

void AWOGAttacker::BeginPlay()
{
	Super::BeginPlay();

	SetAllocatedRaven();
	SetAllocatedMiner();
}

void AWOGAttacker::SetAllocatedRaven()
{
	TArray<AActor*> OutActors;
	FName Tag = FName(*FString::FromInt(PlayerProfile.UserIndex));
	UGameplayStatics::GetAllActorsOfClassWithTag(this, AWOGRaven::StaticClass(), Tag, OutActors);

	if (OutActors.IsEmpty()) return;

	Raven = Cast<AWOGRaven>(OutActors[0]);
}

void AWOGAttacker::SetAllocatedMiner()
{
	TArray<AActor*> OutActors;
	FName Tag = FName(*FString::FromInt(PlayerProfile.UserIndex));
	UGameplayStatics::GetAllActorsOfClassWithTag(this, AWOGMinerGiant::StaticClass(), Tag, OutActors);

	if (OutActors.IsEmpty()) return;

	Miner = Cast<AWOGMinerGiant>(OutActors[0]);
}

void AWOGAttacker::PossessMinion()
{
	if (!bCanPossessMinion && OwnerPC)
	{
		TObjectPtr<UWOGUIManagerSubsystem> UIManager = ULocalPlayer::GetSubsystem<UWOGUIManagerSubsystem>(OwnerPC->GetLocalPlayer());
		if (UIManager)
		{
			UIManager->CreateResourceWarningWidget(FString("Health"));
			UE_LOG(WOGLogSpawn, Error, TEXT("Health too low, can't possess"));
		}
		return;
	}

	OwnerPC = OwnerPC == nullptr ? (TObjectPtr<AWOGPlayerController>) Cast<AWOGPlayerController>(GetController()) : OwnerPC;
	if (!OwnerPC)
	{
		UE_LOG(LogTemp, Error, TEXT("invalid OwnerPC"));
		return;
	}

	if (!CurrentTarget)
	{
		UE_LOG(LogTemp, Error, TEXT("invalid CurrentTargetActor"));
		return;
	}

	OwnerPC->Server_PossessMinion(CurrentTarget);
	TargetComponent->TargetLockOff();

	TObjectPtr<UWOGUIManagerSubsystem> UIManager = ULocalPlayer::GetSubsystem<UWOGUIManagerSubsystem>(OwnerPC->GetLocalPlayer());
	if (UIManager)
	{
		UIManager->RemoveCrosshairWidget();
	}

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
	if (bIsAlternativeModeEnabled) return;
	FVector2D AbilitiesVector = Value.Get<FVector2D>();

	if (AbilitiesVector.X > 0)
	{
		//Button 4/Down pressed
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

		//Check for Cooldown
		if (MagicToEquip && HasMatchingGameplayTag(MagicToEquip->GetMagicData().CooldownTag))
		{
			UE_LOG(LogTemp, Error, TEXT("Cooldown in effect. Can't equip"));
			return;
		}

		//Check for resource
		bool bSucessCheck = false;
		if (AttributeSet && MagicToEquip && MagicToEquip->GetMagicData().Cost > UAbilitySystemBlueprintLibrary::GetFloatAttribute(this, AttributeSet->GetManaAttribute(), bSucessCheck))
		{
			TObjectPtr<UWOGUIManagerSubsystem> UIManager = ULocalPlayer::GetSubsystem<UWOGUIManagerSubsystem>(OwnerPC->GetLocalPlayer());
			if (UIManager)
			{
				UIManager->CreateResourceWarningWidget(FString("Mana"));
				UE_LOG(WOGLogCombat, Error, TEXT("Not enough Mana. Can't equip"));
			}
			return;
		}

		SendAbilityLocalInput(EWOGAbilityInputID::Ability2);
	}
	if (AbilitiesVector.Y < 0)
	{
		//Button 3/Down pressed
		if (!EquipmentManager) return;

		AActor* OutMagic = nullptr;
		EquipmentManager->GetMagicShortcutReference(FName("2"), OutMagic);
		if (!OutMagic) return;

		TObjectPtr<AWOGBaseMagic> MagicToEquip = Cast<AWOGBaseMagic>(OutMagic);
		if (MagicToEquip && MagicToEquip->GetMagicData().AbilityInputType != EAbilityInputType::EAI_Instant) return;

		//Check for Cooldown
		if (MagicToEquip && HasMatchingGameplayTag(MagicToEquip->GetMagicData().CooldownTag))
		{
			UE_LOG(LogTemp, Error, TEXT("Cooldown in effect. Can't equip"));
			return;
		}

		//Check for Resource
		bool bSucessCheck = false;
		if (AttributeSet && MagicToEquip && MagicToEquip->GetMagicData().Cost > UAbilitySystemBlueprintLibrary::GetFloatAttribute(this, AttributeSet->GetManaAttribute(), bSucessCheck))
		{
			TObjectPtr<UWOGUIManagerSubsystem> UIManager = ULocalPlayer::GetSubsystem<UWOGUIManagerSubsystem>(OwnerPC->GetLocalPlayer());
			if (UIManager)
			{
				UIManager->CreateResourceWarningWidget(FString("Mana"));
				UE_LOG(WOGLogCombat, Error, TEXT("Not enough Mana. Can't equip"));
			}
			return;
		}

		SendAbilityLocalInput(EWOGAbilityInputID::Ability3);
	}
}

void AWOGAttacker::Ability2HoldButtonStarted(const FInputActionValue& Value)
{
	if (bIsAlternativeModeEnabled) return;
	AbilityHoldStarted(FName("1"));
}

void AWOGAttacker::Ability2HoldButtonTriggered(const FInputActionValue& Value)
{
	//Button 2/Up pressed

	//Remove hold bar widget
	RemoveHoldProgressBarWidget();

	if (!EquipmentManager) return;
	if (bIsAlternativeModeEnabled) return;

	AActor* OutMagic = nullptr;
	EquipmentManager->GetMagicShortcutReference(FName("1"), OutMagic);
	if (!OutMagic) return;

	TObjectPtr<AWOGBaseMagic> MagicToEquip = Cast<AWOGBaseMagic>(OutMagic);
	if (MagicToEquip && MagicToEquip->GetMagicData().AbilityInputType != EAbilityInputType::EAI_Hold) return;

	//Check for cooldown
	if (MagicToEquip && HasMatchingGameplayTag(MagicToEquip->GetMagicData().CooldownTag))
	{
		UE_LOG(LogTemp, Error, TEXT("Cooldown in effect. Can't equip"));
		return;
	}

	//Check for resource
	bool bSucessCheck = false;
	if (AttributeSet && MagicToEquip && MagicToEquip->GetMagicData().Cost > UAbilitySystemBlueprintLibrary::GetFloatAttribute(this, AttributeSet->GetManaAttribute(), bSucessCheck))
	{
		TObjectPtr<UWOGUIManagerSubsystem> UIManager = ULocalPlayer::GetSubsystem<UWOGUIManagerSubsystem>(OwnerPC->GetLocalPlayer());
		if (UIManager)
		{
			UIManager->CreateResourceWarningWidget(FString("Mana"));
			UE_LOG(WOGLogCombat, Error, TEXT("Not enough Mana. Can't equip"));
		}
		return;
	}

	//Execute ability
	SendAbilityLocalInput(EWOGAbilityInputID::Ability2);
}

void AWOGAttacker::Ability3HoldButtonStarted(const FInputActionValue& Value)
{
	if (bIsAlternativeModeEnabled) return;
	AbilityHoldStarted(FName("2"));
}

void AWOGAttacker::Ability3HoldButtonTriggered(const FInputActionValue& Value)
{
	//Button 3/Down pressed

	//Remove hold bar widget
	RemoveHoldProgressBarWidget();

	if (!EquipmentManager) return;
	if (bIsAlternativeModeEnabled) return;

	AActor* OutMagic = nullptr;
	EquipmentManager->GetMagicShortcutReference(FName("2"), OutMagic);
	if (!OutMagic) return;

	TObjectPtr<AWOGBaseMagic> MagicToEquip = Cast<AWOGBaseMagic>(OutMagic);
	if (MagicToEquip && MagicToEquip->GetMagicData().AbilityInputType != EAbilityInputType::EAI_Hold) return;

	//Check for cooldown
	if (MagicToEquip && HasMatchingGameplayTag(MagicToEquip->GetMagicData().CooldownTag))
	{
		UE_LOG(LogTemp, Error, TEXT("Cooldown in effect. Can't equip"));
		return;
	}

	//Check for resource
	bool bSucessCheck = false;
	if (AttributeSet && MagicToEquip && MagicToEquip->GetMagicData().Cost > UAbilitySystemBlueprintLibrary::GetFloatAttribute(this, AttributeSet->GetManaAttribute(), bSucessCheck))
	{
		TObjectPtr<UWOGUIManagerSubsystem> UIManager = ULocalPlayer::GetSubsystem<UWOGUIManagerSubsystem>(OwnerPC->GetLocalPlayer());
		if (UIManager)
		{
			UIManager->CreateResourceWarningWidget(FString("Mana"));
			UE_LOG(WOGLogCombat, Error, TEXT("Not enough Mana. Can't equip"));
		}
		return;
	}

	//Execute ability
	SendAbilityLocalInput(EWOGAbilityInputID::Ability3);
}

void AWOGAttacker::AlternativeActionPressed(const FInputActionValue& Value)
{
	if (!bIsAlternativeModeEnabled || !EnemyOrderComponent || !EnemyOrderComponent->GetCurrentlySelectedSquad()) return;
	FVector2D OrderAction = Value.Get<FVector2D>();

	//Key 1/Left pressed
	if (OrderAction.X < 0)
	{
		//Attack Target order
		if (CurrentTarget)
		{
			EnemyOrderComponent->Server_SendOrder(EnemyOrderComponent->GetCurrentlySelectedSquad(), EEnemyOrder::EEO_AttackTarget, FTransform(), CurrentTarget);
			EnemyOrderComponent->OnOrderSentDelegate.Broadcast(1);

			FGameplayEventData EventPayload;
			EventPayload.EventTag = TAG_Event_Order_Attack_Target;
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, TAG_Event_Order_Attack_Target, EventPayload);
		}
	}

	//Key 2/Up pressed
	if (OrderAction.Y > 0)
	{
		//Hold order
		FHitResult HitResult;
		FVector Start = FollowCamera->GetComponentLocation() + (FollowCamera->GetForwardVector() * 300.f);
		FVector End = FollowCamera->GetComponentLocation() + (FollowCamera->GetForwardVector() * 50000.f);
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);

		bool bFoundHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECollisionChannel::ECC_Visibility, Params);

		if (bFoundHit)
		{
			FTransform HoldTransform = FTransform();
			HoldTransform.SetLocation(HitResult.Location + FVector(0, 0, 100));
			EnemyOrderComponent->Server_SendOrder(EnemyOrderComponent->GetCurrentlySelectedSquad(), EEnemyOrder::EEO_Hold, HoldTransform);
			EnemyOrderComponent->OnOrderSentDelegate.Broadcast(2);

			FGameplayEventData EventPayload;
			EventPayload.EventTag = TAG_Event_Order_Hold;
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, TAG_Event_Order_Hold, EventPayload);
		}
	}

	//Key 3/Right pressed
	if (OrderAction.X > 0)
	{
		//Attack random order
	
		EnemyOrderComponent->Server_SendOrder(EnemyOrderComponent->GetCurrentlySelectedSquad(), EEnemyOrder::EEO_AttackRandom);
		EnemyOrderComponent->OnOrderSentDelegate.Broadcast(3);

		FGameplayEventData EventPayload;
		EventPayload.EventTag = TAG_Event_Order_Attack_Target;
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, TAG_Event_Order_Attack_Target, EventPayload);
	}

	//Key 4/Down pressed
	if (OrderAction.Y < 0)
	{
		//Follow order
		EnemyOrderComponent->Server_SendOrder(EnemyOrderComponent->GetCurrentlySelectedSquad(), EEnemyOrder::EEO_Follow);
		EnemyOrderComponent->OnOrderSentDelegate.Broadcast(4);

		FGameplayEventData EventPayload;
		EventPayload.EventTag = TAG_Event_Order_Follow;
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, TAG_Event_Order_Follow, EventPayload);
	}
}

void AWOGAttacker::AlternativeModeEnabled(const bool& NewEnabled)
{
	if (!OwnerPC) return;
	bIsAlternativeModeEnabled = NewEnabled;
	UE_LOG(WOGLogCombat, Display, TEXT("AlternativeMode enabled: %s"), bIsAlternativeModeEnabled ? *FString("True") : *FString("False"));

	TObjectPtr<UWOGUIManagerSubsystem> UIManager = ULocalPlayer::GetSubsystem<UWOGUIManagerSubsystem>(OwnerPC->GetLocalPlayer());
	if (!UIManager)
	{
		UE_LOG(WOGLogUI, Error, TEXT("UIManagerSubsystem from AAttacker invalid"));
		return;
	}

	if (bIsAlternativeModeEnabled)
	{
		UIManager->AddSquadOrderWidget();
	}
	else
	{
		UIManager->RemoveSquadOrderWidget();
	}
}

void AWOGAttacker::ChangeActiveSquadActionPressed(const FInputActionValue& Value)
{
	if (!bIsAlternativeModeEnabled || !EnemyOrderComponent) return;
	FVector2D SquadAction = Value.Get<FVector2D>();
	if (SquadAction.X < 0)
	{
		EnemyOrderComponent->DecreaseCurrentlySelectedSquad();
	}
	if (SquadAction.X > 0)
	{
		EnemyOrderComponent->IncreaseCurrentlySelectedSquad();
	}
}

void AWOGAttacker::PossessRaven()
{
	if (!bCanPossessMinion && OwnerPC)
	{
		TObjectPtr<UWOGUIManagerSubsystem> UIManager = ULocalPlayer::GetSubsystem<UWOGUIManagerSubsystem>(OwnerPC->GetLocalPlayer());
		if (UIManager)
		{
			UIManager->CreateResourceWarningWidget(FString("Health"));
			UE_LOG(WOGLogSpawn, Error, TEXT("Health too low, can't possess"));
		}
		return;
	}

	if (!Raven)
	{
		UE_LOG(LogTemp, Error, TEXT("No valid raven reference"));
		return;
	}

	OwnerPC = OwnerPC == nullptr ? (TObjectPtr<AWOGPlayerController>) Cast<AWOGPlayerController>(GetController()) : OwnerPC;
	if (!OwnerPC)
	{
		UE_LOG(LogTemp, Error, TEXT("invalid OwnerPC"));
		return;
	}

	Raven->SetActorRotation(FRotator());
	Raven->bUseControllerRotationPitch = true;
	Raven->bUseControllerRotationYaw = true;

	OwnerPC->Server_PossessMinion(Raven);
	TargetComponent->TargetLockOff();

	TObjectPtr<UWOGUIManagerSubsystem> UIManager = ULocalPlayer::GetSubsystem<UWOGUIManagerSubsystem>(OwnerPC->GetLocalPlayer());
	if (UIManager)
	{
		UIManager->AddRavenMarkerWidget(Raven->SpawnedMarkers.Num());
	}
}

void AWOGAttacker::PossessMiner()
{
	if (!bCanPossessMinion && OwnerPC)
	{
		TObjectPtr<UWOGUIManagerSubsystem> UIManager = ULocalPlayer::GetSubsystem<UWOGUIManagerSubsystem>(OwnerPC->GetLocalPlayer());
		if (UIManager)
		{
			UIManager->CreateResourceWarningWidget(FString("Health"));
			UE_LOG(WOGLogSpawn, Error, TEXT("Health too low, can't possess"));
		}
		return;
	}

	if (!Miner)
	{
		UE_LOG(LogTemp, Error, TEXT("No valid miner reference"));
		return;
	}

	OwnerPC = OwnerPC == nullptr ? (TObjectPtr<AWOGPlayerController>) Cast<AWOGPlayerController>(GetController()) : OwnerPC;
	if (!OwnerPC)
	{
		UE_LOG(LogTemp, Error, TEXT("invalid OwnerPC"));
		return;
	}

	OwnerPC->Server_PossessMinion(Miner);
	TargetComponent->TargetLockOff();
}

void AWOGAttacker::OnHealthAttributeChanged(const FOnAttributeChangeData& Data)
{
	if (Data.NewValue <= (AttributeSet->GetMaxHealth() * 0.2f) && Data.OldValue > (AttributeSet->GetMaxHealth() * 0.2f))
	{
		UE_LOG(WOGLogSpawn, Warning, TEXT("Health below 20%"));

		if (HasAuthority())
		{
			bCanPossessMinion = false;
		}

		if (CurrentExternalMinion)
		{
			ISpawnInterface* SpawnInterface = Cast<ISpawnInterface>(CurrentExternalMinion);
			if (SpawnInterface)
			{
				SpawnInterface->Execute_UnpossessMinion(CurrentExternalMinion);
				UE_LOG(LogTemp, Warning, TEXT("SpawnInterface Unpossess function called from: %s"), *UEnum::GetValueAsString(GetLocalRole()));
			}
		}
		else
		{
			UE_LOG(WOGLogSpawn, Error, TEXT("CurrentExternalMinion invalid"));
		}
	}

	if (Data.NewValue >= (AttributeSet->GetMaxHealth() * 0.2f) && Data.OldValue < (AttributeSet->GetMaxHealth() * 0.2f) && HasAuthority())
	{
		UE_LOG(WOGLogSpawn, Display, TEXT("Health above 20%"));

		bCanPossessMinion = true;
	}

	//Check if health is at 80%
	if (Data.NewValue <= (AttributeSet->GetMaxHealth() * 0.8f) && Data.OldValue > (AttributeSet->GetMaxHealth() * 0.8f) && !IsLocallyControlled())
	{
		UE_LOG(WOGLogSpawn, Display, TEXT("Health at 80%"));
		if (!OwnerPC || !CurrentExternalMinion)
		{
			return;
		}

		TObjectPtr<UWOGUIManagerSubsystem> UIManager = ULocalPlayer::GetSubsystem<UWOGUIManagerSubsystem>(OwnerPC->GetLocalPlayer());
		if (UIManager)
		{
			UIManager->CreateGenericWarningWidget(FString("TakingDamage"));
			UIManager->AddScreenDamageWidget(0);
		}
	}

	//Check if health is at 60%
	if (Data.NewValue <= (AttributeSet->GetMaxHealth() * 0.6f) && Data.OldValue > (AttributeSet->GetMaxHealth() * 0.6f) && !IsLocallyControlled())
	{
		UE_LOG(WOGLogSpawn, Display, TEXT("Health at 60%"));
		if (!OwnerPC || !CurrentExternalMinion)
		{
			return;
		}

		TObjectPtr<UWOGUIManagerSubsystem> UIManager = ULocalPlayer::GetSubsystem<UWOGUIManagerSubsystem>(OwnerPC->GetLocalPlayer());
		if (UIManager)
		{
			UIManager->CreateGenericWarningWidget(FString("TakingDamage"));
			UIManager->AddScreenDamageWidget(1);
		}
	}

	//Check if health is at 40%
	if (Data.NewValue <= (AttributeSet->GetMaxHealth() * 0.4f) && Data.OldValue > (AttributeSet->GetMaxHealth() * 0.4f) && !IsLocallyControlled())
	{
		UE_LOG(WOGLogSpawn, Display, TEXT("Health at 40%"));
		if (!OwnerPC || !CurrentExternalMinion)
		{
			return;
		}

		TObjectPtr<UWOGUIManagerSubsystem> UIManager = ULocalPlayer::GetSubsystem<UWOGUIManagerSubsystem>(OwnerPC->GetLocalPlayer());
		if (UIManager)
		{
			UIManager->CreateGenericWarningWidget(FString("TakingDamage"));
			UIManager->AddScreenDamageWidget(2);
		}
	}
	
	AWOGBaseCharacter::OnHealthAttributeChanged(Data);
}

void AWOGAttacker::SetCurrentExternalMinion(AActor* NewMinion)
{
	if (NewMinion)
	{
		CurrentExternalMinion = NewMinion;
		UE_LOG(WOGLogSpawn, Display, TEXT("CurrentExternalMinion = %s, -> %s"), *GetNameSafe(NewMinion), *UEnum::GetValueAsString(GetLocalRole()));
	}
	else
	{
		CurrentExternalMinion = nullptr;
		UE_LOG(WOGLogSpawn, Display, TEXT("CurrentExternalMinion = nullptr -> %s"), *UEnum::GetValueAsString(GetLocalRole()));
	}

}
