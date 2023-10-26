// Fill out your copyright notice in the Description page of Project Settings.


#include "BasePlayerCharacter.h"
#include "WOG.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Net/UnrealNetwork.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameMode/WOGGameMode.h"
#include "PlayerController/WOGPlayerController.h"
#include "Weapons/WOGBaseWeapon.h"
#include "Kismet/GameplayStatics.h"
#include "AnimInstance/WOGBaseAnimInstance.h"
#include "Sound/SoundCue.h"
#include "ActorComponents/WOGAbilitySystemComponent.h"
#include "GameplayEffectTypes.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayTagContainer.h"
#include "PlayerState/WOGPlayerState.h"
#include "Types/WOGGameplayTags.h"
#include "Components/AGR_EquipmentManager.h"
#include "Components/AGR_InventoryManager.h"
#include "Components/AGRAnimMasterComponent.h"
#include "Libraries/WOGBlueprintLibrary.h"
#include "WOG/Interfaces/BuildingInterface.h"
#include "WOG/Interfaces/AttributesInterface.h"
#include "TargetSystemComponent.h"
#include "Magic/WOGBaseMagic.h"
#include "AbilitySystem/AttributeSets/WOGAttributeSetBase.h"
#include "UI/WOGHoldProgressBar.h"
#include "UI/AutoSettingWidget.h"
#include "Resources/WOGCommonInventory.h"
#include "Subsystems/WOGUIManagerSubsystem.h"

void ABasePlayerCharacter::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	CharacterMI = UMaterialInstanceDynamic::Create(Material, this);
}

ABasePlayerCharacter::ABasePlayerCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	/**
	*Actor components
	*/

	EquipmentManager = CreateDefaultSubobject<UAGR_EquipmentManager>(TEXT("EquipmentManager"));
	EquipmentManager->SetIsReplicated(true);
	InventoryManager = CreateDefaultSubobject<UAGR_InventoryManager>(TEXT("InventoryManager"));
	InventoryManager->SetIsReplicated(true);
	TargetComponent = CreateDefaultSubobject<UTargetSystemComponent>(TEXT("TargetComponent"));


	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	if (TargetComponent)
	{
		TargetComponent->OnTargetLockedOn.AddDynamic(this, &ThisClass::TargetLocked);
		TargetComponent->OnTargetLockedOff.AddDynamic(this, &ThisClass::TargetUnlocked);
	}

	PreviousWeapon = NAME_None;
	PreviousMagic = NAME_None;
	CurrentMagic = NAME_None;

	if (GetMesh())
	{
		GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECR_Ignore);
	}
}

void ABasePlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ABasePlayerCharacter, PlayerProfile);
	DOREPLIFETIME(ABasePlayerCharacter, CurrentTarget);
	DOREPLIFETIME(ABasePlayerCharacter, CurrentMagic);
	DOREPLIFETIME(ABasePlayerCharacter, PreviousMagic);
	DOREPLIFETIME(ABasePlayerCharacter, PreviousWeapon);
	DOREPLIFETIME(ABasePlayerCharacter, CommonInventory);
	DOREPLIFETIME(ABasePlayerCharacter, OwnerPC);
}

void ABasePlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	SetOwner(NewController);

	OwnerPC = Cast<AWOGPlayerController>(GetController());
}

void ABasePlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->ClearAllMappings();
			Subsystem->AddMappingContext(MatchMappingContext, 0);
		}
	}
}

void ABasePlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		//Move:
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::MoveActionPressed);
		//Look:
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ThisClass::LookActionPressed);
		//Jump
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ThisClass::JumpActionPressed);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		//Dodge
		EnhancedInputComponent->BindAction(DodgeAction, ETriggerEvent::Completed, this, &ThisClass::DodgeActionPressed);
		//Sprint
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Triggered, this, &ThisClass::SprintActionPressed);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ThisClass::StopSprinting);
		//Target
		EnhancedInputComponent->BindAction(TargetAction, ETriggerEvent::Completed, this, &ThisClass::TargetActionPressed);
		EnhancedInputComponent->BindAction(CycleTargetAction, ETriggerEvent::Triggered, this, &ThisClass::CycleTargetActionPressed);

		//Equip
		EnhancedInputComponent->BindAction(AbilitiesAction, ETriggerEvent::Triggered, this, &ThisClass::AbilitiesButtonPressed);

		//PrimaryLightAction
		EnhancedInputComponent->BindAction(PrimaryLightAction, ETriggerEvent::Triggered, this, &ThisClass::PrimaryLightButtonPressed);
		EnhancedInputComponent->BindAction(PrimaryHeavyAction, ETriggerEvent::Started, this, &ThisClass::PrimaryHeavyAttackStarted);
		EnhancedInputComponent->BindAction(PrimaryHeavyAction, ETriggerEvent::Ongoing, this, TEXT("PrimaryArmHeavyAttack"));
		EnhancedInputComponent->BindAction(PrimaryHeavyAction, ETriggerEvent::Canceled, this, &ThisClass::PrimaryHeavyAttackCanceled);
		EnhancedInputComponent->BindAction(PrimaryHeavyAction, ETriggerEvent::Triggered, this, &ThisClass::PrimaryExecuteHeavyAttack);
		//SecondaryAction
		EnhancedInputComponent->BindAction(SecondaryAction, ETriggerEvent::Started, this, &ThisClass::SecondaryButtonPressed);
		EnhancedInputComponent->BindAction(SecondaryAction, ETriggerEvent::Triggered, this, &ThisClass::SecondaryButtonReleased);
		//Weapon Ranged
		EnhancedInputComponent->BindAction(WeaponRangedAction, ETriggerEvent::Triggered, this, &ThisClass::WeaponRangedActionPressed);
	}
}

void ABasePlayerCharacter::MoveActionPressed(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (MovementVector.X != 0)
	{
		FGameplayEventData EventPayload;
		EventPayload.EventMagnitude = MovementVector.X;
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, TAG_Event_Movement_Right, EventPayload);
	}
	if (MovementVector.Y != 0)
	{
		FGameplayEventData EventPayload;
		EventPayload.EventMagnitude = MovementVector.Y;
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, TAG_Event_Movement_Forward, EventPayload);
	}
}

void ABasePlayerCharacter::MoveCharacter(const FVector2D& MovementVector)
{
	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ABasePlayerCharacter::LookActionPressed(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (AnimManager && AnimManager->AimOffsetType == EAGR_AimOffsets::Aim)
	{
		//Disable mouse input if character is targeting
	}
	else
	{
		if (Controller != nullptr)
		{
			// add yaw and pitch input to controller
			AddControllerYawInput(LookAxisVector.X);
			AddControllerPitchInput(LookAxisVector.Y);
		}
	}
}

void ABasePlayerCharacter::JumpActionPressed(const FInputActionValue& Value)
{
	SendAbilityLocalInput(EWOGAbilityInputID::Jump);
}

void ABasePlayerCharacter::DodgeActionPressed(const FInputActionValue& Value)
{
	SendAbilityLocalInput(EWOGAbilityInputID::Dodge);
}

void ABasePlayerCharacter::SprintActionPressed()
{
	SendAbilityLocalInput(EWOGAbilityInputID::Sprint);	
}

void ABasePlayerCharacter::StopSprinting()
{
	FGameplayEventData EventPayload;
	EventPayload.EventTag = TAG_Event_Movement_Sprint_Stop;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, TAG_Event_Movement_Sprint_Stop, EventPayload);
}

void ABasePlayerCharacter::TargetActionPressed(const FInputActionValue& Value)
{
	if (HasMatchingGameplayTag(TAG_State_Dead)) return;
	if (HasMatchingGameplayTag(TAG_State_Dodging)) return;

	if (!TargetComponent || HasMatchingGameplayTag(TAG_State_Weapon_Ranged_AOE)) return;
	TargetComponent->TargetActor();
}

void ABasePlayerCharacter::CycleTargetActionPressed(const FInputActionValue& Value)
{
	if (HasMatchingGameplayTag(TAG_State_Dead)) return;

	if (!TargetComponent) return;
	float CycleFloat = Value.Get<float>();

	TargetComponent->TargetActorWithAxisInput(CycleFloat);
}

void ABasePlayerCharacter::WeaponRangedActionPressed(const FInputActionValue& Value)
{
	if (HasMatchingGameplayTag(TAG_State_Weapon_Ranged_AOE) && AbilitySystemComponent.Get())
	{
		AbilitySystemComponent->LocalInputCancel();
		UE_LOG(LogTemp, Warning, TEXT("Cancelled Input"))
	}
	else
	{
		SendAbilityLocalInput(EWOGAbilityInputID::Ranged);
	}
}

void ABasePlayerCharacter::AbilityHoldStarted(const FName& Slot)
{
	if (!EquipmentManager) return;

	AActor* OutMagic = nullptr;
	EquipmentManager->GetMagicShortcutReference(Slot, OutMagic);
	TObjectPtr<AWOGBaseMagic> Magic = Cast<AWOGBaseMagic>(OutMagic);

	if (Magic && HasMatchingGameplayTag(Magic->GetMagicData().CooldownTag))
	{
		UE_LOG(LogTemp, Error, TEXT("Cooldown in effect. Can't equip"));
		return;
	}
	if (!Magic || Magic->GetMagicData().AbilityInputType != EAbilityInputType::EAI_Hold)
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid Magic or not HOLD type"));
		return;
	}

	AddHoldProgressBar();
}

void ABasePlayerCharacter::ConfirmHoldStarted()
{
	TObjectPtr<AWOGBaseMagic> EquippedMagic = UWOGBlueprintLibrary::GetEquippedMagic(this);
	if (EquippedMagic && HasMatchingGameplayTag(EquippedMagic->GetMagicData().CooldownTag))
	{
		UE_LOG(LogTemp, Error, TEXT("Cooldown in effect. Can't equip"));
		return;
	}
	if (!EquippedMagic || EquippedMagic->GetMagicData().AbilityType != EAbilityType::EAT_AOE)
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid Magic or not AOE type"));
		return;
	}

	AddHoldProgressBar();
}

void ABasePlayerCharacter::AbilityHoldButtonElapsed(FInputActionValue ActionValue, float ElapsedTime, float TriggeredTime)
{
	TObjectPtr<AWOGPlayerController> OwnerController = Cast<AWOGPlayerController>(Controller);
	if (!OwnerController || !IsLocallyControlled()) return;

	TObjectPtr<UWOGUIManagerSubsystem> UIManager = ULocalPlayer::GetSubsystem<UWOGUIManagerSubsystem>(OwnerController->GetLocalPlayer());
	if (!UIManager || !UIManager->GetHoldProgressBar())
	{
		return;
	}

	UIManager->GetHoldProgressBar()->TimeHeld = ElapsedTime;
}

void ABasePlayerCharacter::AbilityHoldButtonCanceled(const FInputActionValue& Value)
{
	RemoveHoldProgressBarWidget();
}

void ABasePlayerCharacter::PrimaryLightButtonPressed(const FInputActionValue& Value)
{
	TObjectPtr<AWOGBaseMagic> EquippedMagic = UWOGBlueprintLibrary::GetEquippedMagic(this);
	if (EquippedMagic && EquippedMagic->GetMagicData().AbilityType != EAbilityType::EAT_AOE && AbilitySystemComponent.Get())
	{
		AbilitySystemComponent->LocalInputConfirm();
		UE_LOG(LogTemp, Warning, TEXT("Confirmed Input"));
	}

	TObjectPtr<AWOGBaseWeapon> EquippedWeapon = UWOGBlueprintLibrary::GetEquippedWeapon(this);
	if (EquippedWeapon && HasMatchingGameplayTag(TAG_State_Weapon_Ranged_AOE) && AbilitySystemComponent.Get())
	{
		AbilitySystemComponent->LocalInputConfirm();
		UE_LOG(LogTemp, Warning, TEXT("Confirmed Input"));
		return;
	}

	bool bCanActivateAbility = UWOGBlueprintLibrary::GetEquippedWeapon(this) || UWOGBlueprintLibrary::GetEquippedMagic(this);
	if (bCanActivateAbility)
	{
		SendAbilityLocalInput(EWOGAbilityInputID::AttackLight);
	}
}

void ABasePlayerCharacter::PrimaryHeavyAttackStarted(const FInputActionValue& Value)
{
	ConfirmHoldStarted();
}

void ABasePlayerCharacter::PrimaryArmHeavyAttack(FInputActionValue ActionValue, float ElapsedTime, float TriggeredTime)
{
	if (UWOGBlueprintLibrary::GetEquippedWeapon(this))
	{
		if (ElapsedTime > 0.21f)
		{
			SendAbilityLocalInput(EWOGAbilityInputID::AttackHeavy);
			return;
		}
	}

	TObjectPtr<AWOGPlayerController> OwnerController = Cast<AWOGPlayerController>(Controller);
	if (!OwnerController || !IsLocallyControlled()) return;

	TObjectPtr<UWOGUIManagerSubsystem> UIManager = ULocalPlayer::GetSubsystem<UWOGUIManagerSubsystem>(OwnerController->GetLocalPlayer());
	if (!UIManager || !UIManager->GetHoldProgressBar()) return;

	UIManager->GetHoldProgressBar()->TimeHeld = ElapsedTime;
}

void ABasePlayerCharacter::PrimaryHeavyAttackCanceled(const FInputActionValue& Value)
{
	if (UWOGBlueprintLibrary::GetEquippedWeapon(this))
	{
		FGameplayEventData EventPayload;
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, TAG_Event_Weapon_HeavyAttackCancel, EventPayload);
		return;
	}
	
	RemoveHoldProgressBarWidget();
}

void ABasePlayerCharacter::PrimaryExecuteHeavyAttack(const FInputActionValue& Value)
{
	if (UWOGBlueprintLibrary::GetEquippedWeapon(this))
	{
		FGameplayEventData EventPayload;
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, TAG_Event_Weapon_HeavyAttackExecute, EventPayload);
		return;
	}

	TObjectPtr<AWOGBaseMagic> EquippedMagic = UWOGBlueprintLibrary::GetEquippedMagic(this);
	if (EquippedMagic && EquippedMagic->GetMagicData().AbilityType == EAbilityType::EAT_AOE && AbilitySystemComponent.Get())
	{
		RemoveHoldProgressBarWidget();
		AbilitySystemComponent->LocalInputConfirm();
		UE_LOG(LogTemp, Warning, TEXT("Confirmed Input"))
	}
}

void ABasePlayerCharacter::SecondaryButtonPressed(const FInputActionValue& Value)
{
	if (UWOGBlueprintLibrary::GetEquippedWeapon(this))
	{
		SendAbilityLocalInput(EWOGAbilityInputID::Block);
	}

	bSecondaryButtonPressed = true;
}

void ABasePlayerCharacter::SecondaryButtonReleased(const FInputActionValue& Value)
{
	AActor* OutItem;
	if (!EquipmentManager->GetItemInSlot(NAME_WeaponSlot_Primary, OutItem) || !OutItem) return;

	FGameplayEventData EventPayload;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, TAG_Event_Weapon_Block_Stop, EventPayload);
	GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Purple, FString("StopBlocking"));

	bSecondaryButtonPressed = false;
}

void ABasePlayerCharacter::Server_SetPlayerProfile_Implementation(const FPlayerData& NewPlayerProfile)
{
	PlayerProfile = NewPlayerProfile;
	UpdatePlayerProfile(PlayerProfile);
}

void ABasePlayerCharacter::OnRep_PlayerProfile()
{
	UpdatePlayerProfile(PlayerProfile);
}

void ABasePlayerCharacter::UpdatePlayerProfile_Implementation(const FPlayerData& NewPlayerProfile)
{
	//Set the character meshes
	SetMeshesAndAnimations(NewPlayerProfile.bIsMale, NewPlayerProfile.CharacterIndex);

	//Set the character colors
	SetColors(NewPlayerProfile.PrimaryColor, NewPlayerProfile.SkinColor, NewPlayerProfile.BodyPaintColor, NewPlayerProfile.HairColor);	

	//Set the character default abilities and effects
	SetDefaultAbilitiesAndEffects(NewPlayerProfile.bIsMale, NewPlayerProfile.CharacterIndex);
}

void ABasePlayerCharacter::SetColors(FName Primary, FName Skin, FName BodyPaint, FName HairColor)
{
	UDataTable* PrimaryColorTable = CharacterDataTables.PrimaryColors;
	UDataTable* SkinColorTable = CharacterDataTables.SkinColor;
	UDataTable* BodyPaintColorTable = CharacterDataTables.BodyPaintColor;
	UDataTable* HairColorTable = CharacterDataTables.HairColor;

	if (
		!PrimaryColorTable ||
		!SkinColorTable ||
		!BodyPaintColorTable ||
		!HairColorTable
		)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "Error with data tables");
		return;
	}

	FMaterialColors* PrimaryRow = PrimaryColorTable->FindRow<FMaterialColors>(Primary, FString());
	FMaterialColors* SkinRow = SkinColorTable->FindRow<FMaterialColors>(Skin, FString());
	FMaterialColors* BodyPaintRow = BodyPaintColorTable->FindRow<FMaterialColors>(BodyPaint, FString());
	FMaterialColors* HairRow = HairColorTable->FindRow<FMaterialColors>(HairColor, FString());

	if (
		!PrimaryRow ||
		!SkinRow ||
		!BodyPaintRow ||
		!HairRow
		)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "Error with data table rows");
		return;
	}

	CharacterMI->SetVectorParameterValue(FName("Color_Primary"), PrimaryRow->ColorVector);
	CharacterMI->SetVectorParameterValue(FName("Color_Secondary"), PrimaryRow->ColorVectorSec);
	CharacterMI->SetVectorParameterValue(FName("Color_Skin"), SkinRow->ColorVector);
	CharacterMI->SetVectorParameterValue(FName("Color_Stubble"), (SkinRow->ColorVector*FLinearColor(0.57f, 0.57f, 0.57f, 1.0f)));
	CharacterMI->SetVectorParameterValue(FName("Color_BodyArt"), BodyPaintRow->ColorVector);
	CharacterMI->SetVectorParameterValue(FName("Color_Hair"), HairRow->ColorVector);
}

void ABasePlayerCharacter::SetMeshesAndAnimations(bool bIsMale, FName RowName)
{
	UDataTable* MeshTable = bIsMale ? CharacterDataTables.MaleBody : CharacterDataTables.FemaleBody;

	if (!MeshTable)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "Error with data tables");
		return;
	}

	FCharacterMesh* MeshRow = MeshTable->FindRow<FCharacterMesh>(RowName, FString());

	if (!MeshRow)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "Error with data table rows");
		return;
	}

	MergeParams.bNeedsCpuAccess = true;

	MergeParams.MeshesToMerge.Add(MeshRow->Head);
	MergeParams.MeshesToMerge.Add(MeshRow->Torso);
	MergeParams.MeshesToMerge.Add(MeshRow->Hips);
	MergeParams.MeshesToMerge.Add(MeshRow->ArmUpperLeft);
	MergeParams.MeshesToMerge.Add(MeshRow->ArmUpperRight);
	MergeParams.MeshesToMerge.Add(MeshRow->ArmLowerLeft);
	MergeParams.MeshesToMerge.Add(MeshRow->ArmLowerRight);
	MergeParams.MeshesToMerge.Add(MeshRow->HandLeft);
	MergeParams.MeshesToMerge.Add(MeshRow->HandRight);
	MergeParams.MeshesToMerge.Add(MeshRow->LegLeft);
	MergeParams.MeshesToMerge.Add(MeshRow->LegRight);
	MergeParams.MeshesToMerge.Add(MeshRow->Hair);
	MergeParams.MeshesToMerge.Add(MeshRow->Beard);
	MergeParams.MeshesToMerge.Add(MeshRow->Ears);
	MergeParams.MeshesToMerge.Add(MeshRow->Eyebrows);
	MergeParams.MeshesToMerge.Add(MeshRow->Helmet);

	MergeParams.Skeleton = MeshRow->Skeleton;

	USkeletalMesh* NewMesh = UMeshMergeFunctionLibrary::MergeMeshes(MergeParams);
	if (NewMesh)
	{
		//UE_LOG(LogTemp, Warning, TEXT("%d Amount of LODs"), NewMesh->GetLODNum());
		//NewMesh->GetLODInfo(NewMesh->GetLODNum())->bAllowCPUAccess = true;

		GetMesh()->SetSkinnedAssetAndUpdate(NewMesh, true);
		GetMesh()->SetMaterial(0, CharacterMI);
		GetMesh()->SetMaterial(1, CharacterMI);
		GetMesh()->SetMaterial(2, CharacterMI);
		GetMesh()->SetAnimClass(MeshRow->AnimBP);
		GetMesh()->SetPhysicsAsset(MeshRow->PhysicsAsset);
	}

	UnarmedHurtMontage = MeshRow->UnarmedHurtMontage;
	DodgeMontage = MeshRow->DodgeMontage;
}

void ABasePlayerCharacter::SetDefaultAbilitiesAndEffects(bool bIsMale, FName RowName)
{
	UDataTable* MeshTable = bIsMale ? CharacterDataTables.MaleBody : CharacterDataTables.FemaleBody;

	if (!MeshTable)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "Error with data tables");
		return;
	}

	FCharacterMesh* MeshRow = MeshTable->FindRow<FCharacterMesh>(RowName, FString());

	if (!MeshRow)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "Error with data table rows");
		return;
	}

	DefaultAbilitiesAndEffects.Abilities.Append(MeshRow->DefaultAbilitiesAndEffects.Abilities);
	DefaultAbilitiesAndEffects.Effects.Append(MeshRow->DefaultAbilitiesAndEffects.Effects);
	DefaultAbilitiesAndEffects.Weapons.Append(MeshRow->DefaultAbilitiesAndEffects.Weapons);

	GiveDefaultAbilities();
	ApplyDefaultEffects();

	CharacterData = MeshRow->CharacterData;
	CharacterData.bIsMale = bIsMale;

	FindCommonInventory();
}

void ABasePlayerCharacter::FindCommonInventory()
{
	if (HasAuthority())
	{
		FName Tag = GetCharacterData().bIsAttacker ? FName("Attacker") : FName("Defender");
		TArray<AActor*> OutActors;
		UGameplayStatics::GetAllActorsOfClassWithTag(this, AWOGCommonInventory::StaticClass(), Tag, OutActors);

		if (OutActors.Num())
		{
			CommonInventory = CastChecked<AWOGCommonInventory>(OutActors[0]);
			UE_LOG(LogTemp, Display, TEXT("CommonInventory found: %s"), *GetNameSafe(CommonInventory));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("No Common Inventory found"));
		}
	}
}

void ABasePlayerCharacter::ResetPreviouslyEquippedMaterial()
{
	if (!EquipmentManager)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString("Equipment component invalid"));
		return;
	}

	//Unequip current magic
	if (CurrentMagic != NAME_None)
	{
		AActor* PrimaryMagic = nullptr;
		EquipmentManager->GetItemInSlot(NAME_MagicSlot_MagicPrimary, PrimaryMagic);
		if (PrimaryMagic)
		{
			Server_UnequipMagic(CurrentMagic, PrimaryMagic);
			UE_LOG(LogTemp, Warning, TEXT("Unequipped Magic: %s"), *GetNameSafe(PrimaryMagic));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Equipped magic invalid"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("CurrentMagic FName == NAME_None"));
	}

	//Reequip previous weapon
	if (PreviousWeapon != NAME_None)
	{
		UE_LOG(LogTemp, Warning, TEXT("PreviousWeapon key == %s"), *PreviousWeapon.ToString());
		AActor* OutItem = nullptr;
		EquipmentManager->GetWeaponShortcutReference(PreviousWeapon, OutItem);
		if (OutItem)
		{
			Server_EquipWeapon(PreviousWeapon, OutItem);
			UE_LOG(LogTemp, Warning, TEXT("Equipped Previous Weapon: %s"), *GetNameSafe(OutItem));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Previous weapon invalid"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("PreviousWeapon FName == NAME_None"));
	}

	//Reequip previous magic
	if (PreviousMagic != NAME_None)
	{
		AActor* OutMagic = nullptr;
		EquipmentManager->GetMagicShortcutReference(PreviousMagic, OutMagic);
		if (OutMagic)
		{
			Server_EquipMagic(PreviousMagic, OutMagic);
			UE_LOG(LogTemp, Warning, TEXT("Equipped Previous Magic: %s"), *GetNameSafe(OutMagic));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Previous magic invalid"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("PreviousMagic FName == NAME_None"));
	}
}

void ABasePlayerCharacter::UnequipMagic(const bool& bIsAttacker, const FName& Slot)
{
	if (!EquipmentManager)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString("Equipment component invalid"));
		return;
	}

	FName OtherSlot = Slot == FName("1") ? FName("2") : FName("1");

	AActor* OutMagic = nullptr;
	AActor* PrimaryMagic = nullptr;
	EquipmentManager->GetMagicShortcutReference(Slot, OutMagic);
	EquipmentManager->GetItemInSlot(NAME_MagicSlot_MagicPrimary, PrimaryMagic);
	if (PrimaryMagic && OutMagic && PrimaryMagic == OutMagic)
	{
		Server_UnequipMagic(Slot, PrimaryMagic);
	}
	else if (bIsAttacker)
	{
		EquipmentManager->GetMagicShortcutReference(OtherSlot, OutMagic);
		EquipmentManager->GetItemInSlot(NAME_MagicSlot_MagicPrimary, PrimaryMagic);
		if (PrimaryMagic && OutMagic && PrimaryMagic == OutMagic)
		{
			Server_UnequipMagic(OtherSlot, PrimaryMagic);
		}
	}
}

void ABasePlayerCharacter::UnequipWeapon(const bool& bIsAttacker, const FName& Slot)
{
	if (!EquipmentManager)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString("Equipment component invalid"));
		return;
	}

	FName OtherSlot = Slot == FName("1") ? FName("2") : FName("1");

	AActor* OutItem = nullptr;
	AActor* PrimaryItem = nullptr;
	EquipmentManager->GetWeaponShortcutReference(Slot, OutItem);
	EquipmentManager->GetItemInSlot(NAME_WeaponSlot_Primary, PrimaryItem);

	if (PrimaryItem && OutItem && PrimaryItem == OutItem) //Weapon #1 equipped
	{
		Server_UnequipWeaponSwap(NAME_WeaponSlot_BackMain, PrimaryItem);
	}
	else if(!bIsAttacker) //Weapon #2 equipped && is Defender
	{
		EquipmentManager->GetWeaponShortcutReference(OtherSlot, OutItem);
		EquipmentManager->GetItemInSlot(NAME_WeaponSlot_Primary, PrimaryItem);
		if (PrimaryItem && OutItem && PrimaryItem == OutItem)
		{
			Server_UnequipWeaponSwap(NAME_WeaponSlot_BackSecondary, PrimaryItem);
		}
	}
}

void ABasePlayerCharacter::EquipMagic(const FName& Slot)
{
	if (!EquipmentManager)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString("Equipment component invalid"));
		return;
	}

	AActor* PrimaryMagic = nullptr;
	AActor* OutMagic = nullptr;
	EquipmentManager->GetMagicShortcutReference(Slot, OutMagic);
	EquipmentManager->GetItemInSlot(NAME_MagicSlot_MagicPrimary, PrimaryMagic);
	if (PrimaryMagic && OutMagic && PrimaryMagic == OutMagic)
	{
		Server_UnequipMagic(Slot, PrimaryMagic);
	}
	else if (OutMagic)
	{
		Server_EquipMagic(Slot, OutMagic);
	}
}

void ABasePlayerCharacter::EquipWeapon(const FName& Slot)
{
	AActor* OutItem = nullptr;
	AActor* PrimaryItem = nullptr;
	EquipmentManager->GetWeaponShortcutReference(Slot, OutItem);
	EquipmentManager->GetItemInSlot(NAME_WeaponSlot_Primary, PrimaryItem);
	if (PrimaryItem && OutItem && PrimaryItem == OutItem)
	{
		FGameplayEventData EventPayload;
		EventPayload.EventTag = TAG_Event_Weapon_Unequip;
		EventPayload.OptionalObject = PrimaryItem;
		int32 Key = FCString::Atoi(*Slot.ToString());
		EventPayload.EventMagnitude = Key;
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, TAG_Event_Weapon_Unequip, EventPayload);
	}
	else if (OutItem)
	{
		FGameplayEventData EventPayload;
		EventPayload.EventTag = TAG_Event_Weapon_Equip;
		EventPayload.OptionalObject = OutItem;
		int32 Key = FCString::Atoi(*Slot.ToString());
		EventPayload.EventMagnitude = Key;
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, TAG_Event_Weapon_Equip, EventPayload);
	}
}

void ABasePlayerCharacter::Client_SaveShortcutReferences_Implementation(AActor* InItem, const FGameplayTag& InItemTag, const FName& Key)
{
	if (!InventoryManager || !EquipmentManager) return;

	//Prefer the Actor pointer
	if (IsValid(InItem))
	{
		EquipmentManager->SaveShortcutReference(Key, InItem);
		UE_LOG(LogTemp, Warning, TEXT("Added Item : %s to shortcut key : %s"), *GetNameSafe(InItem), *Key.ToString());
		return;
	}
	//If actor pointer invalid, use tag
	else
	{
		TArray<AActor*> OutItems;
		int32 AmountItems = 0;
		if (InventoryManager->GetAllItemsOfTagSlotType(InItemTag, OutItems, AmountItems))
		{
			if (IsValid(OutItems[0]))
			{
				EquipmentManager->SaveShortcutReference(Key, OutItems[0]);
				UE_LOG(LogTemp, Warning, TEXT("Added Item : %s to shortcut key : %s"), *GetNameSafe(OutItems[0]), *Key.ToString());
				return;
			}
		}
	}
	UE_LOG(LogTemp, Error, TEXT("Item not added to shortcut"));
}

void ABasePlayerCharacter::Server_EquipWeapon_Implementation(const FName& Key, AActor* InWeapon)
{
	if (!InWeapon) return;

	//Determine the back slots names
	FName RelevantBackSlot;
	FName OtherBackSlot;
	if (Key == FName("1"))
	{
		RelevantBackSlot = NAME_WeaponSlot_BackMain;
		OtherBackSlot = NAME_WeaponSlot_BackSecondary;
	}
	else if (Key == FName("2"))
	{
		RelevantBackSlot = NAME_WeaponSlot_BackSecondary;
		OtherBackSlot = NAME_WeaponSlot_BackMain;
	}

	PreviousWeapon = NAME_None;

	//Determine where to equip weapons
	AActor* PrimarySlotActor;
	EquipmentManager->GetItemInSlot(NAME_WeaponSlot_Primary, PrimarySlotActor);
	if (PrimarySlotActor == InWeapon)
	{
		//InWeapon already equipped. Put it on the back
		FText Note;
		EquipmentManager->UnEquipByReference(InWeapon, Note);
		AActor* PreviousItem;
		AActor* NewItem;
		EquipmentManager->EquipItemInSlot(RelevantBackSlot, InWeapon, PreviousItem, NewItem);
		UE_LOG(LogTemp, Display, TEXT("Same as Primary - Equipping to BackMain"));
		return;
	}
	else
	{
		//InWeapon NOT equipped as primary
		//Determine where to equip weapons
		AActor* BackSlotActor;
		EquipmentManager->GetItemInSlot(RelevantBackSlot, BackSlotActor);
		if (BackSlotActor == InWeapon)
		{
			//InWeapon equipped on the back releavant slot
			if (EquipmentManager->GetItemInSlot(NAME_WeaponSlot_Primary, PrimarySlotActor))
			{
				//Primary slot taken. Send Previous weapon to back.
				FText Note;
				EquipmentManager->UnEquipByReference(PrimarySlotActor, Note);
				AActor* PreviousItem;
				AActor* NewItem;
				EquipmentManager->EquipItemInSlot(OtherBackSlot, PrimarySlotActor, PreviousItem, NewItem);


				if (OtherBackSlot == NAME_WeaponSlot_BackMain)
				{
					PreviousWeapon = FName("2");
				}
				else if (OtherBackSlot == NAME_WeaponSlot_BackSecondary)
				{
					PreviousWeapon = FName("1");
				}
			}

			//Equip InWeapon to primary
			FText Note;
			EquipmentManager->UnEquipByReference(InWeapon, Note);
			AActor* PreviousItem;
			AActor* NewItem;
			EquipmentManager->EquipItemInSlot(NAME_WeaponSlot_Primary, InWeapon, PreviousItem, NewItem);
			PreviousWeapon = Key;
			UE_LOG(LogTemp, Display, TEXT("Same as BackMain - Equipping to Primary"));
			return;
		}
		else
		{
			//InWeapon was never equipped before
			AActor* PreviousItem;
			AActor* NewItem;
			EquipmentManager->EquipItemInSlot(RelevantBackSlot, InWeapon, PreviousItem, NewItem);
			UE_LOG(LogTemp, Display, TEXT("Never equipped before - Equipping to Relevant BackSlot"));
			return;
		}
	}
	UE_LOG(LogTemp, Error, TEXT("ERROR WHILE EQUIPPING"));
}

void ABasePlayerCharacter::Server_UnequipWeapon_Implementation(const FName& Key, AActor* InWeapon)
{
	if (!InWeapon) return;

	//Determine the back slots names
	FName BackSlot;
	if (Key == FName("1"))
	{
		BackSlot = NAME_WeaponSlot_BackMain;
	}
	else if (Key == FName("2"))
	{
		BackSlot = NAME_WeaponSlot_BackSecondary;
	}

	PreviousWeapon = NAME_None;

	//Unequip from primary and equip to back
	FText Note;
	EquipmentManager->UnEquipByReference(InWeapon, Note);
	AActor* PreviousItem;
	AActor* NewItem;
	EquipmentManager->EquipItemInSlot(BackSlot, InWeapon, PreviousItem, NewItem);
	UE_LOG(LogTemp, Display, TEXT("WeaponUnequipped"));
	return;
}

void ABasePlayerCharacter::Server_UnequipWeaponSwap_Implementation(const FName& Key, AActor* InWeapon)
{
	FText Note;
	EquipmentManager->UnEquipByReference(InWeapon, Note);
	AActor* PreviousItem;
	AActor* NewItem;
	EquipmentManager->EquipItemInSlot(Key, InWeapon, PreviousItem, NewItem);
}

void ABasePlayerCharacter::Server_EquipMagic_Implementation(const FName& Key, AActor* InMagic)
{
	if (!InMagic) return;

	//Determine the back slots names
	FName RelevantBackSlot;
	FName OtherBackSlot;
	if (Key == FName("1"))
	{
		RelevantBackSlot = NAME_MagicSlot_MagicBackMain;
		OtherBackSlot = NAME_MagicSlot_MagicBackSecondary;
	}
	else if (Key == FName("2"))
	{
		RelevantBackSlot = NAME_MagicSlot_MagicBackSecondary;
		OtherBackSlot = NAME_MagicSlot_MagicBackMain;
	}

	PreviousMagic = NAME_None;

	//Determine where to equip magic
	AActor* PrimarySlotActor;
	EquipmentManager->GetItemInSlot(NAME_MagicSlot_MagicPrimary, PrimarySlotActor);
	if (PrimarySlotActor == InMagic)
	{
		//InMagic already equipped. Put it on the back
		FText Note;
		EquipmentManager->UnEquipByReference(InMagic, Note);
		AActor* PreviousItem;
		AActor* NewItem;
		EquipmentManager->EquipItemInSlot(RelevantBackSlot, InMagic, PreviousItem, NewItem);
		CurrentMagic = NAME_None;
		UE_LOG(LogTemp, Display, TEXT("Same magic as MagicPrimary - Equipping to MagicBackMain"));
		return;
	}
	else
	{
		//InMagic NOT equipped as primary
		//Determine where to equip weapons
		AActor* BackSlotActor;
		EquipmentManager->GetItemInSlot(RelevantBackSlot, BackSlotActor);
		if (BackSlotActor == InMagic)
		{
			//InWeapon equipped on the back releavant slot
			if (EquipmentManager->GetItemInSlot(NAME_MagicSlot_MagicPrimary, PrimarySlotActor))
			{
				//Primary slot taken. Send Previous weapon to back.
				FText Note;
				EquipmentManager->UnEquipByReference(PrimarySlotActor, Note);
				AActor* PreviousItem;
				AActor* NewItem;
				EquipmentManager->EquipItemInSlot(OtherBackSlot, PrimarySlotActor, PreviousItem, NewItem);

				if (OtherBackSlot == NAME_MagicSlot_MagicBackMain)
				{
					PreviousMagic = FName("1");
				}
				else if (OtherBackSlot == NAME_MagicSlot_MagicBackSecondary)
				{
					PreviousMagic = FName("2");
				}
			}

			//Equip InWeapon to primary
			FText Note;
			EquipmentManager->UnEquipByReference(InMagic, Note);
			AActor* PreviousItem;
			AActor* NewItem;
			EquipmentManager->EquipItemInSlot(NAME_MagicSlot_MagicPrimary, InMagic, PreviousItem, NewItem);
			CurrentMagic = Key;
			UE_LOG(LogTemp, Display, TEXT("Same as MagicBackMain - Equipping to MagicPrimary"));
			return;
		}
		else
		{
			//InWeapon was never equipped before
			AActor* PreviousItem;
			AActor* NewItem;
			EquipmentManager->EquipItemInSlot(RelevantBackSlot, InMagic, PreviousItem, NewItem);
			UE_LOG(LogTemp, Display, TEXT("Magic Never equipped before - Equipping to MagicRelevantBack Slot"));
			return;
		}
	}
	UE_LOG(LogTemp, Error, TEXT("ERROR WHILE EQUIPPING"));
}

void ABasePlayerCharacter::Server_UnequipMagic_Implementation(const FName& Key, AActor* InWeapon)
{
	if (!InWeapon) return;

	//Determine the back slots names
	FName BackSlot;
	if (Key == FName("1"))
	{
		BackSlot = NAME_MagicSlot_MagicBackMain;
	}
	else if (Key == FName("2"))
	{
		BackSlot = NAME_MagicSlot_MagicBackSecondary;
	}

	//Unequip from primary and equip to back
	FText Note;
	EquipmentManager->UnEquipByReference(InWeapon, Note);
	AActor* PreviousItem;
	AActor* NewItem;
	EquipmentManager->EquipItemInSlot(BackSlot, InWeapon, PreviousItem, NewItem);
	CurrentMagic == NAME_None;
	return;
}

void ABasePlayerCharacter::HandleStateElimmed(AController* InstigatedBy)
{
	if (!HasAuthority()) return;

	WOGGameMode = WOGGameMode == nullptr ?	GetWorld()->GetAuthGameMode<AWOGGameMode>() : WOGGameMode;

	TObjectPtr<AWOGPlayerController> Victim = Cast<AWOGPlayerController>(GetController());
	TObjectPtr<AWOGPlayerController> Attacker = Cast<AWOGPlayerController>(InstigatedBy);

	if (!WOGGameMode || !Victim || !Attacker)
	{
		if (!WOGGameMode)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, FString("NoGameMode"));
		}
		if (!Victim)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, FString("NoOwnerPC"));
		}
		if (!Attacker)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, FString("NoAttackerPC"));
		}
		return;
	}

	WOGGameMode->PlayerEliminated(this, Victim, Attacker);
}

void ABasePlayerCharacter::ProcessHit(FHitResult Hit, UPrimitiveComponent* WeaponMesh)
{
	if (!WeaponMesh) return;

	AWOGBaseWeapon* AttackerWeapon = WeaponMesh->GetOwner() ? Cast<AWOGBaseWeapon>(WeaponMesh->GetOwner()) : nullptr;
	if (!AttackerWeapon)
	{
		UE_LOG(LogTemp, Error, TEXT("No Attacker Weapon"));
	}

	AWOGBaseCharacter* AttackerCharacter = AttackerWeapon ? Cast<AWOGBaseCharacter>(AttackerWeapon->GetOwner()) : nullptr;
	if (!AttackerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("No Attacker Character"));
		return;
	}

	//Get the Damage to apply values:
	float DamageToApply = 0.f;
	if (HasAuthority())
	{
		if (AttackerCharacter->HasMatchingGameplayTag(TAG_State_Weapon_AttackLight) && AttackerWeapon)
		{
			DamageToApply = AttackerWeapon->GetWeaponData().BaseDamage +
				(AttackerWeapon->GetWeaponData().BaseDamage * AttackerWeapon->GetWeaponData().DamageMultiplier) +
				(AttackerWeapon->GetWeaponData().BaseDamage * (AttackerWeapon->GetComboStreak() * AttackerWeapon->GetWeaponData().ComboDamageMultiplier));
		}
		if (AttackerCharacter->HasMatchingGameplayTag(TAG_State_Weapon_AttackHeavy) && AttackerWeapon)
		{
			DamageToApply = AttackerWeapon->GetWeaponData().BaseDamage +
				(AttackerWeapon->GetWeaponData().BaseDamage * AttackerWeapon->GetWeaponData().DamageMultiplier) +
				(AttackerWeapon->GetWeaponData().BaseDamage * AttackerWeapon->GetWeaponData().HeavyDamageMultiplier);
		}

		UE_LOG(LogTemp, Warning, TEXT("Base DamageToApply : %f"), DamageToApply);

		float StrenghtMultiplier = AttributeSet->GetStrengthMultiplier();
		DamageToApply *= StrenghtMultiplier;
		UE_LOG(LogTemp, Warning, TEXT("DamageToApply after StrengthMultiplier : %f"), DamageToApply);
	}

	//Check if we hit build and apply build damage
	TObjectPtr<IBuildingInterface> BuildInterface = Cast<IBuildingInterface>(Hit.GetActor());
	if (BuildInterface && HasAuthority())
	{
		BuildInterface->Execute_DealDamage(Hit.GetActor(), DamageToApply);
		UE_LOG(LogTemp, Warning, TEXT("Build damaged with %f"), DamageToApply);
		return;
	}

	//Check if we hit other character
	TObjectPtr<IAttributesInterface> AttributesInterface = Cast<IAttributesInterface>(Hit.GetActor());
	if (AttributesInterface)
	{
		bool FoundAttribute;
		float DamageReduction = UAbilitySystemBlueprintLibrary::GetFloatAttribute(Hit.GetActor(), AttributeSet->GetDamageReductionAttribute(), FoundAttribute);
		DamageToApply *= (1 - DamageReduction);
		UE_LOG(LogTemp, Warning, TEXT("DamageToApply after DamageReduction of %f : %f"), DamageReduction, DamageToApply);

		AttributesInterface->Execute_BroadcastHit(Hit.GetActor(), AttackerCharacter, Hit, DamageToApply, AttackerWeapon);
	}
}

void ABasePlayerCharacter::ProcessMagicHit(const FHitResult& Hit, const FMagicDataTable& MagicData)
{
	if (!Hit.GetActor())
	{
		UE_LOG(LogTemp, Error, TEXT("No Victim actor"));
		return;
	}

	//Get the Damage to apply values:
	float DamageToApply = 0.f;;
	if (HasAuthority())
	{
		DamageToApply = MagicData.Value * MagicData.ValueMultiplier;
		UE_LOG(LogTemp, Warning, TEXT("Base DamageToApply : %f"), DamageToApply);

		float StrenghtMultiplier = AttributeSet->GetStrengthMultiplier();
		DamageToApply *= StrenghtMultiplier;
		UE_LOG(LogTemp, Warning, TEXT("DamageToApply after StrengthMultiplier : %f"), DamageToApply);
	}

	//Check if we hit build and apply build damage
	TObjectPtr<IBuildingInterface> BuildInterface = Cast<IBuildingInterface>(Hit.GetActor());
	if (BuildInterface && HasAuthority())
	{
		BuildInterface->Execute_DealDamage(Hit.GetActor(), DamageToApply);
		UE_LOG(LogTemp, Warning, TEXT("Build damaged with %f"), DamageToApply);
		return;
	}

	//Check if we hit other character
	TObjectPtr<IAttributesInterface> AttributesInterface = Cast<IAttributesInterface>(Hit.GetActor());
	if (AttributesInterface)
	{
		AttributesInterface->Execute_BroadcastMagicHit(Hit.GetActor(), this, Hit, MagicData);
	}
}

void ABasePlayerCharacter::BroadcastHit_Implementation(AActor* AgressorActor, const FHitResult& Hit, const float& DamageToApply, AActor* InstigatorWeapon)
{
	//Handle early returns
	if (HasMatchingGameplayTag(TAG_State_Dead)) return;
	if (HasMatchingGameplayTag(TAG_State_Dodging)) return;
	
	if (!AgressorActor || !InstigatorWeapon) return;
	AWOGBaseWeapon* EquippedWeapon = UWOGBlueprintLibrary::GetEquippedWeapon(this);
	AWOGBaseCharacter* AgressorCharacter = Cast<AWOGBaseCharacter>(AgressorActor);
	AWOGBaseWeapon* AgressorWeapon = Cast<AWOGBaseWeapon>(InstigatorWeapon);

	//Handle more early returns
	if (!EquippedWeapon)
	{
		UE_LOG(LogTemp, Error, TEXT("No Valid Equipped weapon"));
	}
	if (!AgressorCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("No valid Agressor Character"));
		return;
	}
	if (!AgressorWeapon)
	{
		UE_LOG(LogTemp, Error, TEXT("No valid Agressor Weapon"));
		return;
	}

	//Store the last hit result
	LastHitResult = Hit;
	float LocalDamageToApply = DamageToApply;

	//Handle Ranged Weapon Throw Hit
	if (AgressorCharacter->HasMatchingGameplayTag(TAG_State_Weapon_Ranged_Throw) || AgressorCharacter->HasMatchingGameplayTag(TAG_State_Weapon_Ranged_AOE))
	{
		//Victim hit by shield throw
		if (AgressorWeapon->GetWeaponData().WeaponTag.MatchesTag(TAG_Inventory_Weapon_Shield))
		{
			FGameplayEventData EventPayload;
			EventPayload.EventTag = AgressorWeapon->GetWeaponData().RangedTag;
			EventPayload.EventMagnitude = AgressorWeapon->GetWeaponData().StunDuration;
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, AgressorWeapon->GetWeaponData().RangedTag, EventPayload);
			UE_LOG(LogTemp, Warning, TEXT("Shield throw hit and applied: %s during %f seconds"), *AgressorWeapon->GetWeaponData().RangedTag.ToString(), AgressorWeapon->GetWeaponData().StunDuration);

			FGameplayCueParameters CueParams;
			CueParams.Location = Hit.ImpactPoint;
			CueParams.EffectCauser = AgressorCharacter;
			AbilitySystemComponent->ExecuteGameplayCueLocal(TAG_Cue_Weapon_BodyHit, CueParams);
		}

		//Victim hit by dual weapon throw
		if (AgressorWeapon->GetWeaponData().WeaponTag.MatchesTag(TAG_Inventory_Weapon_DualWield))
		{
			FGameplayEventData EventPayload;
			EventPayload.EventTag = AgressorWeapon->GetWeaponData().RangedTag;
			EventPayload.EventMagnitude = AgressorWeapon->GetWeaponData().StunDuration;
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, AgressorWeapon->GetWeaponData().RangedTag, EventPayload);
			UE_LOG(LogTemp, Warning, TEXT("Weapon throw hit and applied: %s during %f seconds"), *AgressorWeapon->GetWeaponData().RangedTag.ToString(), AgressorWeapon->GetWeaponData().StunDuration);

			FGameplayCueParameters CueParams;
			CueParams.Location = Hit.ImpactPoint;
			CueParams.EffectCauser = AgressorCharacter;
			AbilitySystemComponent->ExecuteGameplayCueLocal(TAG_Cue_Weapon_BodyHit, CueParams);
		}

		//Victim hit by two handed ranged attack
		if (AgressorWeapon->GetWeaponData().WeaponTag.MatchesTag(TAG_Inventory_Weapon_TwoHanded))
		{
			FGameplayEventData EventPayload;
			EventPayload.EventTag = AgressorWeapon->GetWeaponData().RangedTag;
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, AgressorWeapon->GetWeaponData().RangedTag, EventPayload);
			UE_LOG(LogTemp, Warning, TEXT("Weapon AOE hit and applied: %s"), *AgressorWeapon->GetWeaponData().RangedTag.ToString());
		}
	}

	//Handle parrying for agressor
	if (HasMatchingGameplayTag(TAG_State_Weapon_Parry) && AgressorCharacter->HasMatchingGameplayTag(TAG_State_Weapon_AttackLight) && IsHitFrontal(60.f, this, FVector::Zero(), AgressorActor))
	{
		//Handle parry on the agressor character

		FGameplayEventData EventPayload;
		EventPayload.EventTag = EquippedWeapon->GetWeaponData().ParryTag;
		EventPayload.EventMagnitude = EquippedWeapon->GetWeaponData().StunDuration;
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(AgressorActor, EquippedWeapon->GetWeaponData().ParryTag, EventPayload);
		UE_LOG(LogTemp, Warning, TEXT("Agressor was parried"));

		//Apply the block impact cue
		FGameplayCueParameters CueParams;
		CueParams.Location = Hit.ImpactPoint;
		CueParams.EffectCauser = AgressorCharacter;
		AbilitySystemComponent->ExecuteGameplayCueLocal(TAG_Cue_Weapon_Block_Impact, CueParams);
		return;
	}

	//Handle blocked hits for victim and agressor
	if (HasMatchingGameplayTag(TAG_State_Weapon_Block) && IsHitFrontal(60.f, this, FVector::Zero(), AgressorActor))
	{
		if (AgressorCharacter->HasMatchingGameplayTag(TAG_State_Weapon_AttackHeavy))
		{
			//Attacker used heavy attack on victim while guarding:
			//Handle knockback on victim

			FGameplayEventData EventKnockbackPayload;
			EventKnockbackPayload.EventTag = EquippedWeapon->GetWeaponData().BlockImpactHeavyTag;
			EventKnockbackPayload.EventMagnitude = EquippedWeapon->GetWeaponData().StunDuration;
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, EquippedWeapon->GetWeaponData().BlockImpactHeavyTag, EventKnockbackPayload);
			UE_LOG(LogTemp, Warning, TEXT("Impact HEAVY applied: %s"), *EquippedWeapon->GetWeaponData().BlockImpactHeavyTag.ToString());
			return;
		}

		if (AgressorCharacter->HasMatchingGameplayTag(TAG_State_Weapon_AttackLight))
		{
			//Attacker used light attack on victim while guarding:
			//Regular impact on the victim 

			FGameplayEventData EventPayload;
			EventPayload.EventMagnitude = DamageToApply;
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, TAG_Event_Weapon_Block_Impact_Light, EventPayload);
			UE_LOG(LogTemp, Warning, TEXT("Impact LIGHT"));
			
			if (EquippedWeapon->GetWeaponData().WeaponTag.MatchesTag(TAG_Inventory_Weapon_DualWield))
			{
				LocalDamageToApply = DamageToApply * 0.2;
				UE_LOG(LogTemp, Warning, TEXT("Blocked with DualWield. Will receive damage"));
			}
			else
			{
				LocalDamageToApply = 0.f;
				UE_LOG(LogTemp, Warning, TEXT("Blocked with other weapon. No damage incoming"));
			}
		}

		//Apply the block impact cue
		FGameplayCueParameters CueParams;
		CueParams.Location = Hit.ImpactPoint;
		CueParams.EffectCauser = AgressorCharacter;
		AbilitySystemComponent->ExecuteGameplayCueLocal(TAG_Cue_Weapon_Block_Impact, CueParams);
	}

	// Handle weapon clashes when victim and agressor attack at the same time
	if (AgressorCharacter->HasMatchingGameplayTag(TAG_State_Weapon_AttackLight) && HasMatchingGameplayTag(TAG_State_Weapon_AttackLight) && IsHitFrontal(60.f, this, FVector::Zero(), AgressorActor))
	{
		//Apply knockback to agressor 
		FGameplayEventData EventPayload;
		EventPayload.EventTag = TAG_Event_Debuff_Stagger;
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(AgressorActor, TAG_Event_Debuff_Knockback, EventPayload);
		UE_LOG(LogTemp, Warning, TEXT("Agressor knockback"));

		//Apply knockback to victim
		FGameplayEventData EventVictimPayload;
		EventVictimPayload.EventTag = TAG_Event_Debuff_Stagger;
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, TAG_Event_Debuff_Knockback, EventVictimPayload);
		UE_LOG(LogTemp, Warning, TEXT("Victim knockback"));
		return;
	}

	//Handle unguarded hit to victim
	if (!HasMatchingGameplayTag(TAG_State_Weapon_Block) && !HasMatchingGameplayTag(TAG_State_Weapon_Parry))
	{
		if (IsHitFrontal(60.f, this, FVector::Zero(), InstigatorWeapon) && AgressorCharacter->HasMatchingGameplayTag(TAG_State_Weapon_AttackHeavy))
		{
			//Send event KO to victim
			FGameplayEventData EventKOPayload;
			EventKOPayload.EventTag = TAG_Event_Debuff_KO;
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, TAG_Event_Debuff_KO, EventKOPayload);
		}

		if (AgressorCharacter->HasMatchingGameplayTag(TAG_State_Weapon_AttackLight))
		{
			//Send Event light HitReact to victim
			FGameplayEventData EventHitReactPayload;
			EventHitReactPayload.EventTag = TAG_Event_Debuff_HitReact;
			EventHitReactPayload.Instigator = InstigatorWeapon;
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, TAG_Event_Debuff_HitReact, EventHitReactPayload);

			FGameplayCueParameters CueParams;
			CueParams.Location = Hit.ImpactPoint;
			CueParams.EffectCauser = AgressorCharacter;
			AbilitySystemComponent->ExecuteGameplayCueLocal(TAG_Cue_Weapon_BodyHit, CueParams);
		}
	}

	//Apply damage to victim if authority
	if (HasAuthority() && AgressorCharacter && AbilitySystemComponent.Get() && AgressorWeapon->GetWeaponData().WeaponDamageEffect)
	{
		FGameplayEffectContextHandle DamageContext = AbilitySystemComponent.Get()->MakeEffectContext();
		DamageContext.AddInstigator(AgressorCharacter, AgressorWeapon);
		DamageContext.AddHitResult(Hit);

		FGameplayEffectSpecHandle OutSpec = AbilitySystemComponent->MakeOutgoingSpec(AgressorWeapon->GetWeaponData().WeaponDamageEffect, 1, DamageContext);
		UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(OutSpec, FGameplayTag::RequestGameplayTag(TEXT("Damage.Attribute.Health")), -LocalDamageToApply);
		AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*OutSpec.Data);
		UE_LOG(LogTemp, Error, TEXT("Damage applied to %s : %f"), *GetNameSafe(this), LocalDamageToApply);
	}
}

void ABasePlayerCharacter::BroadcastMagicHit_Implementation(AActor* AgressorActor, const FHitResult& Hit, const FMagicDataTable& AgressorMagicData)
{
	//Handle early returnswg
	if (HasMatchingGameplayTag(TAG_State_Dead)) return;
	if (HasMatchingGameplayTag(TAG_State_Dodging)) return;

	if (!AgressorActor) return;
	TObjectPtr<AWOGBaseWeapon> EquippedWeapon = UWOGBlueprintLibrary::GetEquippedWeapon(this);
	TObjectPtr<AWOGBaseCharacter> AgressorCharacter = Cast<AWOGBaseCharacter>(AgressorActor);

	//Handle more early returns and warnings
	if (!AgressorCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("No valid Agressor Character"));
		return;
	}

	//Store the last hit result and calculate damage
	LastHitResult = Hit;
	float LocalDamageToApply = AgressorMagicData.Value * AgressorMagicData.ValueMultiplier;

	bool FoundAttribute;
	float DamageReduction = UAbilitySystemBlueprintLibrary::GetFloatAttribute(Hit.GetActor(), AttributeSet->GetDamageReductionAttribute(), FoundAttribute);
	LocalDamageToApply *= (1 - DamageReduction);
	UE_LOG(LogTemp, Warning, TEXT("DamageToApply after DamageReduction of %f : %f"), DamageReduction, LocalDamageToApply);

	//Apply secondary effect
	if (UKismetSystemLibrary::IsValidClass(AgressorMagicData.SecondaryEffect))
	{
		FGameplayEffectContextHandle SecondaryContext = AbilitySystemComponent.Get()->MakeEffectContext();
		SecondaryContext.AddInstigator(AgressorCharacter, AgressorCharacter);
		ApplyGameplayEffectToSelf(AgressorMagicData.SecondaryEffect, SecondaryContext, AgressorMagicData.SecondaryEffectDuration);
	}

	//Handle AOE KO
	if (AgressorMagicData.AbilityType == EAbilityType::EAT_AOE)
	{
		//Send event KO to victim
		FGameplayEventData EventKOPayload;
		EventKOPayload.EventTag = TAG_Event_Debuff_KO;
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, TAG_Event_Debuff_KO, EventKOPayload);
	}

	//Handle blocked hits for victim and agressor
	if (HasMatchingGameplayTag(TAG_State_Weapon_Block) && IsHitFrontal(60.f, this, Hit.ImpactPoint, nullptr))
	{
		//Condition to sort out different types of magic
		if (AgressorMagicData.AbilityType == EAbilityType::EAT_Projectile)
		{
			//Attacker used light attack on victim while guarding:
			//Regular impact on the victim 
			FGameplayEventData EventPayload;
			EventPayload.EventMagnitude = AgressorMagicData.Value;
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, TAG_Event_Weapon_Block_Impact_Light, EventPayload);

			LocalDamageToApply = 0.f;

			//Apply the block impact cue
			FGameplayCueParameters CueParams;
			CueParams.Location = Hit.ImpactPoint;
			CueParams.EffectCauser = AgressorCharacter;
			AbilitySystemComponent->ExecuteGameplayCueLocal(TAG_Cue_Weapon_Block_Impact, CueParams);
		}
	}

	//Handle unguarded hit to victim
	if (!HasMatchingGameplayTag(TAG_State_Weapon_Block) && !HasMatchingGameplayTag(TAG_State_Weapon_Parry))
	{
		//Condition to sort out different types of magic
		if (AgressorMagicData.AbilityType == EAbilityType::EAT_Projectile)
		{
			//Send Event light HitReact to victim
			FGameplayEventData EventHitReactPayload;
			EventHitReactPayload.EventTag = TAG_Event_Debuff_HitReact;
			EventHitReactPayload.Instigator = AgressorCharacter;
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, TAG_Event_Debuff_HitReact, EventHitReactPayload);

			FGameplayCueParameters CueParams;
			CueParams.Location = Hit.ImpactPoint;
			CueParams.EffectCauser = AgressorCharacter;
			AbilitySystemComponent->ExecuteGameplayCueLocal(TAG_Cue_Weapon_BodyHit, CueParams);
		}
	}

	//Apply damage to victim if authority
	if (HasAuthority() && AgressorCharacter && AbilitySystemComponent.Get() && AgressorMagicData.DamageEffect)
	{
		FGameplayEffectContextHandle DamageContext = AbilitySystemComponent.Get()->MakeEffectContext();
		DamageContext.AddInstigator(AgressorCharacter, AgressorCharacter);
		DamageContext.AddHitResult(Hit);

		FGameplayEffectSpecHandle OutSpec = AbilitySystemComponent->MakeOutgoingSpec(AgressorMagicData.DamageEffect, 1, DamageContext);
		UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(OutSpec, FGameplayTag::RequestGameplayTag(TEXT("Damage.Attribute.Health")), -LocalDamageToApply);
		AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*OutSpec.Data);
		UE_LOG(LogTemp, Error, TEXT("Damage applied to %s : %f"), *GetNameSafe(this), LocalDamageToApply);
	}
}

void ABasePlayerCharacter::TargetLocked(AActor* NewTarget)
{
	if (!NewTarget)
	{
		GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Red, FString::Printf(TEXT("Target invalid")));
		return;
	}

	Server_SetCurrentTarget(NewTarget);

	ToggleStrafeMovement(true);
}

void ABasePlayerCharacter::TargetUnlocked(AActor* OldTarget)
{
	Server_SetCurrentTarget();

	ToggleStrafeMovement(false);
}

void ABasePlayerCharacter::Server_SetCurrentTarget_Implementation(AActor* NewTarget)
{
	CurrentTarget = NewTarget;
}

void ABasePlayerCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	/*
	* Create default Pickaxe and Woodaxe
	*/
	if (HasAuthority())
	{
		FTimerHandle TimerHandle;
		float Delay = 2.f;
		GetWorldTimerManager().SetTimer(TimerHandle, this, &ThisClass::CreateDefaultTools, Delay);
	}
}

void ABasePlayerCharacter::Elim(bool bPlayerLeftGame)
{
	Multicast_Elim(bPlayerLeftGame);
}

void ABasePlayerCharacter::Multicast_Elim_Implementation(bool bPlayerLeftGame)
{

	GetMesh()->SetCollisionProfileName(FName("Ragdoll"));
	GetMesh()->SetAllBodiesSimulatePhysics(true);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
	FVector ImpulseDirection = LastHitDirection.GetSafeNormal() * -45000.f;
	GetMesh()->AddImpulse(ImpulseDirection);
	TargetComponent->TargetLockOff();
	if(OwnerPC)	OwnerPC->SetDefaultPawn(nullptr);
	

	/*
	**Handle respawn timer
	*/
	GetWorld()->GetTimerManager().SetTimer(ElimTimer, this, &ThisClass::ElimTimerFinished, ElimDelay);
}

void ABasePlayerCharacter::ElimTimerFinished()
{
	WOGGameMode = WOGGameMode == nullptr ? GetWorld()->GetAuthGameMode<AWOGGameMode>() : WOGGameMode;
	APlayerController* PlayerController = Cast<APlayerController>(Controller);
	if (WOGGameMode && PlayerController)
	{
		WOGGameMode->RequestRespawn(this, PlayerController);
	}
}

void ABasePlayerCharacter::Server_DropWeapon_Implementation(const FName& Key)
{
	if (!EquipmentManager) return;
	AActor* OutItem = nullptr;
	EquipmentManager->GetWeaponShortcutReference(Key, OutItem);

	if (!OutItem)
	{
		UE_LOG(LogTemp, Error, TEXT("NO VALID ITEM REFERENCE AT KEY : %s"), *Key.ToString());
		return;
	}

	TObjectPtr<AWOGBaseWeapon> Weapon = Cast<AWOGBaseWeapon>(OutItem);
	if(!Weapon)	return;

	Weapon->DropWeapon();
}

void ABasePlayerCharacter::Server_DropMagic_Implementation(const FName& Key)
{
	if (!EquipmentManager) return;
	AActor* OutItem = nullptr;
	EquipmentManager->GetMagicShortcutReference(Key, OutItem);

	if (!OutItem)
	{
		UE_LOG(LogTemp, Error, TEXT("NO VALID ITEM REFERENCE AT KEY : %s"), *Key.ToString());
		return;
	}

	TObjectPtr<AWOGBaseMagic> Magic = Cast<AWOGBaseMagic>(OutItem);
	if (!Magic)	return;

	Magic->DropMagic();
}

void ABasePlayerCharacter::Server_StoreWeapons_Implementation()
{
	StoreWeapon(FName("1"));
	StoreWeapon(FName("2"));
	RestoreTools();
}

void ABasePlayerCharacter::Server_RestoreWeapons_Implementation()
{
	StoreTool(FName("1"));
	StoreTool(FName("2"));
	RestoreWeapons();
}

void ABasePlayerCharacter::RestoreTools()
{
	if (!InventoryManager) return;

	TArray<AActor*> OutItems;
	int32 Amount = 0;
	InventoryManager->GetAllItemsOfTagSlotType(TAG_Inventory_Tool, OutItems, Amount);

	if (!OutItems.Num()) return;
	for (auto OutItem : OutItems)
	{
		TObjectPtr<AWOGBaseWeapon> Weapon = Cast<AWOGBaseWeapon>(OutItem);
		if (!Weapon) return;

		Weapon->RestoreWeapon(this);
	}
}

void ABasePlayerCharacter::StoreTool(const FName& Key)
{
	if (!EquipmentManager) return;
	AActor* OutItem = nullptr;
	EquipmentManager->GetWeaponShortcutReference(Key, OutItem);

	if (!OutItem)
	{
		UE_LOG(LogTemp, Error, TEXT("NO VALID ITEM REFERENCE AT KEY : %s"), *Key.ToString());
		return;
	}

	TObjectPtr<AWOGBaseWeapon> Weapon = Cast<AWOGBaseWeapon>(OutItem);
	if (!Weapon) return;

	Weapon->StoreWeapon(Key, this);
}

void ABasePlayerCharacter::StoreWeapon(const FName& Key)
{
	if (!EquipmentManager) return;
	AActor* OutItem = nullptr;
	EquipmentManager->GetWeaponShortcutReference(Key, OutItem);

	if (!OutItem)
	{
		UE_LOG(LogTemp, Error, TEXT("NO VALID ITEM REFERENCE AT KEY : %s"), *Key.ToString());
		return;
	}

	TObjectPtr<AWOGBaseWeapon> Weapon = Cast<AWOGBaseWeapon>(OutItem);
	if (!Weapon) return;

	Weapon->StoreWeapon(Key, this);
}

void ABasePlayerCharacter::RestoreWeapons()
{
	if (!InventoryManager) return;

	TArray<AActor*> OutItems;
	int32 Amount = 0;
	InventoryManager->GetAllItemsOfTagSlotType(TAG_Inventory_Weapon, OutItems, Amount);

	if (!OutItems.Num()) return;
	for (auto OutItem : OutItems)
	{
		TObjectPtr<AWOGBaseWeapon> Weapon = Cast<AWOGBaseWeapon>(OutItem);
		if (!Weapon) return;

		Weapon->RestoreWeapon(this);
	}
}

void ABasePlayerCharacter::CreateDefaultTools()
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;

	if (DefaultPickaxeClass)
	{
		TObjectPtr<AWOGBaseWeapon> DefaultPickaxe = GetWorld()->SpawnActor<AWOGBaseWeapon>(DefaultPickaxeClass, FTransform(), SpawnParams);
		if (DefaultPickaxe)
		{
			InventoryManager->AddItemToInventoryDirectly(DefaultPickaxe);
		}
	}

	if (DefaultWoodaxeClass)
	{
		TObjectPtr<AWOGBaseWeapon> DefaultWoodaxe = GetWorld()->SpawnActor<AWOGBaseWeapon>(DefaultWoodaxeClass, FTransform(), SpawnParams);
		if (DefaultWoodaxe)
		{
			InventoryManager->AddItemToInventoryDirectly(DefaultWoodaxe);
		}
	}

	RestoreTools();
}
