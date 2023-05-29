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
#include "LockOnTargetComponent.h"
#include "TargetingHelperComponent.h"
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
#include "Libraries/WOGBlueprintLibrary.h"
#include "WOG/Interfaces/BuildingInterface.h"
#include "WOG/Interfaces/AttributesInterface.h"


void ABasePlayerCharacter::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	CharacterMI = UMaterialInstanceDynamic::Create(Material, this);
}

// Sets default values
ABasePlayerCharacter::ABasePlayerCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	/**
	*Actor components
	*/

	LockOnTarget = CreateDefaultSubobject<ULockOnTargetComponent>(TEXT("LockOnTargetComponent"));
	LockOnTarget->SetIsReplicated(true);
	TargetAttractor = CreateDefaultSubobject<UTargetingHelperComponent>(TEXT("TargetAttractor"));
	TargetAttractor->SetIsReplicated(true);
	EquipmentManager = CreateDefaultSubobject<UAGR_EquipmentManager>(TEXT("EquipmentManager"));
	EquipmentManager->SetIsReplicated(true);
	InventoryManager = CreateDefaultSubobject<UAGR_InventoryManager>(TEXT("InventoryManager"));
	InventoryManager->SetIsReplicated(true);


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

	CharacterState = ECharacterState::ECS_Unnoccupied;
	bIsTargeting = false;

	if (LockOnTarget)
	{
		LockOnTarget->OnTargetLocked.AddDynamic(this, &ThisClass::TargetLocked);
		LockOnTarget->OnTargetUnlocked.AddDynamic(this, &ThisClass::TargetUnlocked);
		LockOnTarget->OnTargetNotFound.AddDynamic(this, &ThisClass::TargetNotFound);
	}
}

void ABasePlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ABasePlayerCharacter, PlayerProfile);
}

void ABasePlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
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

// Called to bind functionality to input
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
		EnhancedInputComponent->BindAction(PrimaryHeavyAction, ETriggerEvent::Ongoing, this, TEXT("PrimaryArmHeavyAttack"));
		EnhancedInputComponent->BindAction(PrimaryHeavyAction, ETriggerEvent::Canceled, this, &ThisClass::PrimaryHeavyAttackCanceled);
		EnhancedInputComponent->BindAction(PrimaryHeavyAction, ETriggerEvent::Triggered, this, &ThisClass::PrimaryExecuteHeavyAttack);
		//SecondaryAction
		EnhancedInputComponent->BindAction(SecondaryAction, ETriggerEvent::Started, this, &ThisClass::SecondaryButtonPressed);
		EnhancedInputComponent->BindAction(SecondaryAction, ETriggerEvent::Triggered, this, &ThisClass::SecondaryButtonReleased);
	}
}

void ABasePlayerCharacter::MoveActionPressed(const FInputActionValue& Value)
{
	if (HasMatchingGameplayTag(TAG_State_Dead)) return;
	if (HasMatchingGameplayTag(TAG_State_Dodging)) return;
	if (CharacterState == ECharacterState::ECS_Staggered) return;

	FVector2D MovementVector = Value.Get<FVector2D>();
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

	if (bIsTargeting)
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
	if (CharacterState == ECharacterState::ECS_Staggered) return;

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
	if (CharacterState == ECharacterState::ECS_Staggered) return;

	if (!LockOnTarget) return;
	LockOnTarget->EnableTargeting();
}

void ABasePlayerCharacter::CycleTargetActionPressed(const FInputActionValue& Value)
{
	if (HasMatchingGameplayTag(TAG_State_Dead)) return;

	if (!LockOnTarget) return;
	float CycleFloat = Value.Get<float>();
	FVector2D CycleVector = FVector2D(CycleFloat, 0.f);

	LockOnTarget->SwitchTargetManual(CycleVector);
}

void ABasePlayerCharacter::PrimaryLightButtonPressed(const FInputActionValue& Value)
{
	if (HasMatchingGameplayTag(TAG_State_Dead)) return;
	if (HasMatchingGameplayTag(TAG_State_Dodging)) return;
	if (CharacterState == ECharacterState::ECS_Staggered) return;

	if (!EquipmentManager) return;
	AActor* OutItem;
	if (EquipmentManager->GetItemInSlot(NAME_WeaponSlot_Primary, OutItem))
	{
		SendAbilityLocalInput(EWOGAbilityInputID::AttackLight);
	}

	if (AbilitySystemComponent.Get())
	{
		AbilitySystemComponent->LocalInputConfirm();
		UE_LOG(LogTemp, Warning, TEXT("Confirmed Input"))
	}
}

void ABasePlayerCharacter::PrimaryArmHeavyAttack(FInputActionValue ActionValue, float ElapsedTime, float TriggeredTime)
{
	if (HasMatchingGameplayTag(TAG_State_Dead)) return;
	if (HasMatchingGameplayTag(TAG_State_Dodging)) return;
	if (CharacterState == ECharacterState::ECS_Staggered) return;

	AActor* OutItem;
	if (!EquipmentManager->GetItemInSlot(NAME_WeaponSlot_Primary, OutItem) || !OutItem) return;
	
	if (ElapsedTime > 0.21f)
	{
		SendAbilityLocalInput(EWOGAbilityInputID::AttackHeavy);
	}
}

void ABasePlayerCharacter::PrimaryHeavyAttackCanceled(const FInputActionValue& Value)
{
	if (HasMatchingGameplayTag(TAG_State_Dead)) return;
	if (HasMatchingGameplayTag(TAG_State_Dodging)) return;
	if (CharacterState == ECharacterState::ECS_Staggered) return;

	AActor* OutItem;
	if (!EquipmentManager->GetItemInSlot(NAME_WeaponSlot_Primary, OutItem) || !OutItem) return;

	FGameplayEventData EventPayload;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, TAG_Event_Weapon_HeavyAttackCancel, EventPayload);
}

void ABasePlayerCharacter::PrimaryExecuteHeavyAttack(const FInputActionValue& Value)
{
	if (HasMatchingGameplayTag(TAG_State_Dead)) return;
	if (HasMatchingGameplayTag(TAG_State_Dodging)) return;
	if (CharacterState == ECharacterState::ECS_Staggered) return;

	AActor* OutItem;
	if (!EquipmentManager->GetItemInSlot(NAME_WeaponSlot_Primary, OutItem) || !OutItem) return;

	FGameplayEventData EventPayload;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, TAG_Event_Weapon_HeavyAttackExecute, EventPayload);
}

void ABasePlayerCharacter::SecondaryButtonPressed(const FInputActionValue& Value)
{
	if (HasMatchingGameplayTag(TAG_State_Dead)) return;
	if (HasMatchingGameplayTag(TAG_State_Dodging)) return;
	if (CharacterState == ECharacterState::ECS_Staggered) return;

	AActor* OutItem;
	if (!EquipmentManager->GetItemInSlot(NAME_WeaponSlot_Primary, OutItem) || !OutItem) return;

	SendAbilityLocalInput(EWOGAbilityInputID::Block);
}

void ABasePlayerCharacter::SecondaryButtonReleased(const FInputActionValue& Value)
{
	if (HasMatchingGameplayTag(TAG_State_Dead)) return;
	if (HasMatchingGameplayTag(TAG_State_Dodging)) return;
	if (CharacterState == ECharacterState::ECS_Staggered) return;

	AActor* OutItem;
	if (!EquipmentManager->GetItemInSlot(NAME_WeaponSlot_Primary, OutItem) || !OutItem) return;

	FGameplayEventData EventPayload;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, TAG_Event_Weapon_Block_Stop, EventPayload);
	GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Purple, FString("StopBlocking"));
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
		if (InventoryManager->GetAllItemsOfTagSlotType(InItemTag, OutItems))
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
			}

			//Equip InWeapon to primary
			FText Note;
			EquipmentManager->UnEquipByReference(InWeapon, Note);
			AActor* PreviousItem;
			AActor* NewItem;
			EquipmentManager->EquipItemInSlot(NAME_WeaponSlot_Primary, InWeapon, PreviousItem, NewItem);
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

	//Unequip from primary and equip to back
	FText Note;
	EquipmentManager->UnEquipByReference(InWeapon, Note);
	AActor* PreviousItem;
	AActor* NewItem;
	EquipmentManager->EquipItemInSlot(BackSlot, InWeapon, PreviousItem, NewItem);
	UE_LOG(LogTemp, Display, TEXT("WeaponUnequipped"));
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

void ABasePlayerCharacter::HandleStateStaggered()
{
	UAnimInstance* CharacterAnimInstance = GetMesh()->GetAnimInstance();
	if (!CharacterAnimInstance) return;

	AWOGBaseWeapon* EquippedWeapon = UWOGBlueprintLibrary::GetEquippedWeapon(this);
	if (EquippedWeapon && EquippedWeapon->GetWeaponData().BlockMontage)
	{
		CharacterAnimInstance->Montage_Play(EquippedWeapon->GetWeaponData().BlockMontage, 1.f);
		CharacterAnimInstance->Montage_JumpToSection(FName("Knockback"));
	}
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
		AttributesInterface->Execute_BroadcastHit(Hit.GetActor(), AttackerCharacter, Hit, DamageToApply, AttackerWeapon);
		UE_LOG(LogTemp, Warning, TEXT("%s damaged with %s, by %s, in the amount : %f"), *GetNameSafe(Hit.GetActor()), *GetNameSafe(AttackerWeapon), *GetNameSafe(AttackerCharacter), DamageToApply);
	}
}

void ABasePlayerCharacter::BroadcastHit_Implementation(AActor* AgressorActor, const FHitResult& Hit, const float& DamageToApply, AActor* InstigatorWeapon)
{
	if (HasMatchingGameplayTag(TAG_State_Dead)) return;
	if (HasMatchingGameplayTag(TAG_State_Dodging)) return;

	//Handle block hits && stagger && parry
	if (HasMatchingGameplayTag(TAG_State_Weapon_Block))
	{
		if (!AgressorActor || !IsHitFrontal(60.f, this, AgressorActor)) return;

		AWOGBaseWeapon* EquippedWeapon = UWOGBlueprintLibrary::GetEquippedWeapon(this);
		AWOGBaseCharacter* AgressorCharacter = Cast<AWOGBaseCharacter>(AgressorActor);

		if (EquippedWeapon && EquippedWeapon->GetCanParry() && AgressorCharacter && !AgressorCharacter->HasMatchingGameplayTag(TAG_State_Weapon_AttackHeavy))
		{

			//Handle Parry sequence here:
			//Victim handle parry anim:
			FGameplayEventData EventParryPayload;
			EventParryPayload.EventTag = TAG_Event_Weapon_Block_Parry;
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, TAG_Event_Weapon_Block_Parry, EventParryPayload);
			UE_LOG(LogTemp, Warning, TEXT("Parry"));

			//Attacker being staggered:
			FGameplayEventData EventPayload;
			EventPayload.EventTag = TAG_Event_Debuff_Stagger;
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(AgressorCharacter, TAG_Event_Debuff_Stagger, EventPayload);
			UE_LOG(LogTemp, Warning, TEXT("Staggered"));
			return;
		}

		if (InstigatorWeapon && AgressorCharacter && AgressorCharacter->HasMatchingGameplayTag(TAG_State_Weapon_AttackHeavy))
		{
			//Attacker used heavy attack on victim:
			//Handle knockback

			FGameplayEventData EventKnockbackPayload;
			EventKnockbackPayload.EventTag = TAG_Event_Weapon_Block_Knockback;
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, TAG_Event_Weapon_Block_Knockback, EventKnockbackPayload);
			UE_LOG(LogTemp, Warning, TEXT("Knockback"));
			return;
		}

		//Regular impact on the defender 
		FGameplayEventData EventPayload;
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, TAG_Event_Weapon_Block_Impact, EventPayload);
		UE_LOG(LogTemp, Warning, TEXT("Impact"));
		return;
	}

	// Handle weapon clashes when both players are attacking at the same time
	if (HasMatchingGameplayTag(TAG_State_Weapon_AttackLight))
	{
		if (IsHitFrontal(50.f, this, AgressorActor))
		{
			if (AgressorActor)
			{
				FGameplayEventData EventPayload;
				EventPayload.EventTag = TAG_Event_Debuff_Stagger;
				UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(AgressorActor, TAG_Event_Debuff_Stagger, EventPayload);
				UE_LOG(LogTemp, Warning, TEXT("Agressor Staggered"));
			}

			FGameplayEventData EventPayload;
			EventPayload.EventTag = TAG_Event_Debuff_Stagger;
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, TAG_Event_Debuff_Stagger, EventPayload);
			UE_LOG(LogTemp, Warning, TEXT("Victim Staggered"));
			return;
		}
	}


	//TO-DO refactor this multicast into GAS ability
	TObjectPtr<AWOGBaseWeapon> Weapon = Cast<AWOGBaseWeapon>(InstigatorWeapon);
	if (Weapon)
	{
		Multicast_HandleCosmeticHit(ECosmeticHit::ECH_BodyHit, Hit, InstigatorWeapon->GetActorLocation(), Weapon);
	}

	//Apply damage to character if authority
	TObjectPtr<AWOGBaseCharacter> AgressorCharacter = Cast<AWOGBaseCharacter>(AgressorActor);
	if (HasAuthority() && AgressorCharacter && AbilitySystemComponent.Get() && Weapon && Weapon->GetWeaponData().WeaponDamageEffect)
	{
		FGameplayEffectContextHandle DamageContext = AbilitySystemComponent.Get()->MakeEffectContext();
		DamageContext.AddInstigator(AgressorCharacter, Weapon);
		DamageContext.AddHitResult(Hit);

		FGameplayEffectSpecHandle OutSpec = AbilitySystemComponent->MakeOutgoingSpec(Weapon->GetWeaponData().WeaponDamageEffect, 1, DamageContext);
		UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(OutSpec, FGameplayTag::RequestGameplayTag(TEXT("Damage.Attribute.Health")), -DamageToApply);
		AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*OutSpec.Data);
	}
}

void ABasePlayerCharacter::HandleCosmeticBodyHit(const FHitResult& Hit, const FVector& WeaponLocation, const AWOGBaseWeapon* InstigatorWeapon)
{
	FName HitDirection = CalculateHitDirection(Hit, InstigatorWeapon->GetOwnerCharacter()->GetActorLocation());

	if (IsHitFrontal(60.f, this, InstigatorWeapon) && InstigatorWeapon->GetOwnerCharacter()->HasMatchingGameplayTag(TAG_State_Weapon_AttackHeavy))
	{
		//TO-DO Apply proper KO effect here through GAS
		PlayHitReactMontage(FName("KO"));
	}
	else
	{
		PlayHitReactMontage(HitDirection);
	}

	AWOGBaseWeapon* EquippedWeapon = UWOGBlueprintLibrary::GetEquippedWeapon(this);
	if (EquippedWeapon && EquippedWeapon->GetWeaponData().HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, EquippedWeapon->GetWeaponData().HitSound, GetActorLocation());
	}
}

void ABasePlayerCharacter::PlayHitReactMontage(FName Section)
{
	UAnimInstance* CharacterAnimInstance = GetMesh()->GetAnimInstance();
	if (!CharacterAnimInstance) return;

	AWOGBaseWeapon* EquippedWeapon = UWOGBlueprintLibrary::GetEquippedWeapon(this);
	if (EquippedWeapon && EquippedWeapon->GetWeaponData().HurtMontage)
	{
		CharacterAnimInstance->Montage_Play(EquippedWeapon->GetWeaponData().HurtMontage, 1.f);
		CharacterAnimInstance->Montage_JumpToSection(Section);
	}

	if (UnarmedHurtMontage)
	{
		CharacterAnimInstance->Montage_Play(UnarmedHurtMontage, 1.f);
		CharacterAnimInstance->Montage_JumpToSection(Section);
	}
}

void ABasePlayerCharacter::HandleCosmeticBlock(const AWOGBaseWeapon* InstigatorWeapon)
{
	AWOGBaseWeapon* EquippedWeapon = UWOGBlueprintLibrary::GetEquippedWeapon(this);
	if (!EquippedWeapon) return;

	if (EquippedWeapon->GetWeaponData().BlockSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, EquippedWeapon->GetWeaponData().BlockSound, GetActorLocation());
	}

	if (EquippedWeapon->GetWeaponData().BlockMontage)
	{
		UAnimInstance* CharacterAnimInstance = GetMesh()->GetAnimInstance();
		if (!CharacterAnimInstance) return;

		if(InstigatorWeapon && InstigatorWeapon->GetOwnerCharacter() && InstigatorWeapon->GetOwnerCharacter()->HasMatchingGameplayTag(TAG_State_Weapon_AttackLight))
		{
			CharacterAnimInstance->Montage_Play(EquippedWeapon->GetWeaponData().BlockMontage, 1.f);
			CharacterAnimInstance->Montage_JumpToSection(FName("Impact"));
		}
		else if (InstigatorWeapon && InstigatorWeapon->GetOwnerCharacter() && InstigatorWeapon->GetOwnerCharacter()->HasMatchingGameplayTag(TAG_State_Weapon_AttackHeavy))
		{
			FGameplayEventData EventPayload;
			EventPayload.EventTag = TAG_Event_Debuff_Stagger;
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, TAG_Event_Debuff_Stagger, EventPayload);
			UE_LOG(LogTemp, Warning, TEXT("Staggered"));
		}
	}
}

void ABasePlayerCharacter::TargetLocked(UTargetingHelperComponent* Target, FName Socket)
{
	if (Target)
	{
		AActor* TargetOwner = Target->GetOwner();
		if (!TargetOwner)
		{
			GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Red, FString::Printf(TEXT("Target invalid")));
			return;
		}

		CurrentTarget = TargetOwner;
		bIsTargeting = true;
		if (!GetCharacterMovement()) return;

		GetCharacterMovement()->bOrientRotationToMovement = false; // Character doesn't move in the direction of input...
		GetCharacterMovement()->MaxWalkSpeed = 500;	//Sets the maximum run speed
	}
}

void ABasePlayerCharacter::TargetUnlocked(UTargetingHelperComponent* UnlockedTarget, FName Socket)
{
	CurrentTarget = nullptr;
	bIsTargeting = false;
	if (!GetCharacterMovement()) return;

	GetCharacterMovement()->MaxWalkSpeed = 500.f;	//Sets the maximum run speed
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character doesn't move in the direction of input...
}

void ABasePlayerCharacter::TargetNotFound()
{

}

void ABasePlayerCharacter::Elim(bool bPlayerLeftGame)
{
	Multicast_Elim(bPlayerLeftGame);
}

void ABasePlayerCharacter::Multicast_Elim_Implementation(bool bPlayerLeftGame)
{

	GetMesh()->SetCollisionProfileName(FName("Ragdoll"));
	GetMesh()->SetAllBodiesSimulatePhysics(true);
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
	FVector ImpulseDirection = LastHitDirection.GetSafeNormal() * -45000.f;
	GetMesh()->AddImpulse(ImpulseDirection);
	LockOnTarget->ClearTargetManual();
	TargetAttractor->bCanBeCaptured = false;
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