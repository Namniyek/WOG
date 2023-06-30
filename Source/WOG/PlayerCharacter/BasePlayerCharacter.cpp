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
}

void ABasePlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ABasePlayerCharacter, PlayerProfile);
	DOREPLIFETIME(ABasePlayerCharacter, CurrentTarget);
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
		//Weapon Ranged
		EnhancedInputComponent->BindAction(WeaponRangedAction, ETriggerEvent::Triggered, this, &ThisClass::WeaponRangedActionPressed);
	}
}

void ABasePlayerCharacter::MoveActionPressed(const FInputActionValue& Value)
{
	if (HasMatchingGameplayTag(TAG_State_Dead)) return;
	if (HasMatchingGameplayTag(TAG_State_Dodging)) return;
	if (HasMatchingGameplayTag(TAG_State_Debuff_Knockback)) return;
	if (HasMatchingGameplayTag(TAG_State_Debuff_KO)) return;
	if (HasMatchingGameplayTag(TAG_State_Debuff_Stagger)) return;
	if (HasMatchingGameplayTag(TAG_State_Debuff_Stun)) return;

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

	if (!TargetComponent) return;
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
	SendAbilityLocalInput(EWOGAbilityInputID::Ranged);
}

void ABasePlayerCharacter::PrimaryLightButtonPressed(const FInputActionValue& Value)
{
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
	AActor* OutItem;
	if (!EquipmentManager->GetItemInSlot(NAME_WeaponSlot_Primary, OutItem) || !OutItem) return;
	
	if (ElapsedTime > 0.21f)
	{
		SendAbilityLocalInput(EWOGAbilityInputID::AttackHeavy);
	}
}

void ABasePlayerCharacter::PrimaryHeavyAttackCanceled(const FInputActionValue& Value)
{
	AActor* OutItem;
	if (!EquipmentManager->GetItemInSlot(NAME_WeaponSlot_Primary, OutItem) || !OutItem) return;

	FGameplayEventData EventPayload;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, TAG_Event_Weapon_HeavyAttackCancel, EventPayload);
}

void ABasePlayerCharacter::PrimaryExecuteHeavyAttack(const FInputActionValue& Value)
{
	AActor* OutItem;
	if (!EquipmentManager->GetItemInSlot(NAME_WeaponSlot_Primary, OutItem) || !OutItem) return;

	FGameplayEventData EventPayload;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, TAG_Event_Weapon_HeavyAttackExecute, EventPayload);
}

void ABasePlayerCharacter::SecondaryButtonPressed(const FInputActionValue& Value)
{
	AActor* OutItem;
	if (!EquipmentManager->GetItemInSlot(NAME_WeaponSlot_Primary, OutItem) || !OutItem) return;

	SendAbilityLocalInput(EWOGAbilityInputID::Block);

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

	//UE_LOG(LogTemp, Warning, TEXT("%s damaged with %s, by %s, in the amount : %f"), *GetNameSafe(Hit.GetActor()), *GetNameSafe(InstigatorWeapon), *GetNameSafe(AgressorActor), DamageToApply);

	//Handle Ranged Weapon Throw Hit
	if (AgressorCharacter->HasMatchingGameplayTag(TAG_State_Weapon_Ranged))
	{
		//Victim hit by shield throw
		if (AgressorWeapon->GetWeaponData().WeaponTag.MatchesTag(TAG_Inventory_Weapon_Shield))
		{
			FGameplayEventData EventPayload;
			EventPayload.EventTag = AgressorWeapon->GetWeaponData().RangedTag;
			EventPayload.EventMagnitude = AgressorWeapon->GetWeaponData().StunDuration;
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, AgressorWeapon->GetWeaponData().RangedTag, EventPayload);
			UE_LOG(LogTemp, Warning, TEXT("Shield throw hit and applied: %s during %f seconds"), *AgressorWeapon->GetWeaponData().RangedTag.ToString(), AgressorWeapon->GetWeaponData().StunDuration);
		}

		//Victim hit by dual weapon throw
		if (AgressorWeapon->GetWeaponData().WeaponTag.MatchesTag(TAG_Inventory_Weapon_DualWield))
		{
			FGameplayEventData EventPayload;
			EventPayload.EventTag = AgressorWeapon->GetWeaponData().RangedTag;
			EventPayload.EventMagnitude = AgressorWeapon->GetWeaponData().StunDuration;
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, AgressorWeapon->GetWeaponData().RangedTag, EventPayload);
			UE_LOG(LogTemp, Warning, TEXT("Weapon throw hit and applied: %s during %f seconds"), *AgressorWeapon->GetWeaponData().RangedTag.ToString(), AgressorWeapon->GetWeaponData().StunDuration);
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
	if (HasMatchingGameplayTag(TAG_State_Weapon_Parry) && AgressorCharacter->HasMatchingGameplayTag(TAG_State_Weapon_AttackLight) && IsHitFrontal(60.f, this, AgressorActor))
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
		AbilitySystemComponent->ExecuteGameplayCue(TAG_Cue_Weapon_Block_Impact, CueParams);
		return;
	}

	//Handle blocked hits for victim and agressor
	if (HasMatchingGameplayTag(TAG_State_Weapon_Block) && IsHitFrontal(60.f, this, AgressorActor))
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
		AbilitySystemComponent->ExecuteGameplayCue(TAG_Cue_Weapon_Block_Impact, CueParams);
	}

	// Handle weapon clashes when victim and agressor attack at the same time
	if (AgressorCharacter->HasMatchingGameplayTag(TAG_State_Weapon_AttackLight) && HasMatchingGameplayTag(TAG_State_Weapon_AttackLight) && IsHitFrontal(60.f, this, AgressorActor))
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
		if (IsHitFrontal(60.f, this, InstigatorWeapon) && AgressorCharacter->HasMatchingGameplayTag(TAG_State_Weapon_AttackHeavy))
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
			AbilitySystemComponent->ExecuteGameplayCue(TAG_Cue_Weapon_BodyHit, CueParams);
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