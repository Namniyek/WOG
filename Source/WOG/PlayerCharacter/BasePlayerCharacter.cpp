// Fill out your copyright notice in the Description page of Project Settings.


#include "BasePlayerCharacter.h"
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
#include "WOG/ActorComponents/WOGAttributesComponent.h"
#include "WOG/GameMode/WOGGameMode.h"
#include "WOG/PlayerController/WOGPlayerController.h"
#include "WOG/Weapons/WOGBaseWeapon.h"
#include "WOG/ActorComponents/WOGCombatComponent.h"


void ABasePlayerCharacter::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	CharacterMI = UMaterialInstanceDynamic::Create(Material, this);
}

// Sets default values
ABasePlayerCharacter::ABasePlayerCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	/**
	*Actor components
	*/

	LockOnTarget = CreateDefaultSubobject<ULockOnTargetComponent>(TEXT("LockOnTargetComponent"));
	LockOnTarget->SetIsReplicated(true);
	TargetAttractor = CreateDefaultSubobject<UTargetingHelperComponent>(TEXT("TargetAttractor"));
	TargetAttractor->SetIsReplicated(true);
	Attributes = CreateDefaultSubobject<UWOGAttributesComponent>(TEXT("AttributesComponent"));
	Attributes->SetIsReplicated(true);
	Combat = CreateDefaultSubobject<UWOGCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);


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
	DOREPLIFETIME(ABasePlayerCharacter, CharacterState);
}

void ABasePlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
	//if (TargetAttractor)
	//{
	//	TargetAttractor->UpdateDesiredMesh(Head);
	//	TargetAttractor->AddSocket(FName("head"));
	//	TargetAttractor->RemoveSocket();
	//}

	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &ThisClass::ReceiveDamage);
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
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::Move);
		//Look:
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ThisClass::Look);
		//Jump
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		//Dodge
		EnhancedInputComponent->BindAction(DodgeAction, ETriggerEvent::Completed, this, &ThisClass::Dodge);
		//Sprint
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Triggered, this, &ThisClass::Sprint);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ThisClass::StopSprinting);
		//Target
		EnhancedInputComponent->BindAction(TargetAction, ETriggerEvent::Completed, this, &ThisClass::Target);
		//Equip
		EnhancedInputComponent->BindAction(EquipAction, ETriggerEvent::Triggered, this, &ThisClass::Equip);
	}
}

void ABasePlayerCharacter::Move(const FInputActionValue& Value)
{
	if (CharacterState == ECharacterState::ECS_Elimmed) return;
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

void ABasePlayerCharacter::Look(const FInputActionValue& Value)
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

void ABasePlayerCharacter::Dodge(const FInputActionValue& Value)
{
	if (CharacterState == ECharacterState::ECS_Elimmed) return;
	if (CharacterState!=ECharacterState::ECS_Dodging)
	{
		Server_SetCharacterState(ECharacterState::ECS_Dodging);
	}
}

void ABasePlayerCharacter::StopDodging()
{
	if (CharacterState == ECharacterState::ECS_Dodging)
	{
		Server_SetCharacterState(ECharacterState::ECS_Unnoccupied);
	}
}

void ABasePlayerCharacter::Sprint()
{
	if (CharacterState == ECharacterState::ECS_Elimmed) return;
	if (!bIsTargeting)
	{
		Server_SetCharacterState(ECharacterState::ECS_Sprinting);
	}
}

void ABasePlayerCharacter::StopSprinting()
{
	Server_SetCharacterState(ECharacterState::ECS_Unnoccupied);
}

void ABasePlayerCharacter::Target(const FInputActionValue& Value)
{
	if (CharacterState == ECharacterState::ECS_Elimmed) return;
	if (!LockOnTarget) return;
	LockOnTarget->EnableTargeting();
}

void ABasePlayerCharacter::Equip(const FInputActionValue& Value)
{
	if (!Combat)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString("Combat component invalid"));
		return;
	}
	Combat->EquipWeapon();
}

void ABasePlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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
	SetMeshes(NewPlayerProfile.bIsMale, NewPlayerProfile.CharacterIndex);

	//Set the character colors
	SetColors(NewPlayerProfile.PrimaryColor, NewPlayerProfile.SkinColor, NewPlayerProfile.BodyPaintColor, NewPlayerProfile.HairColor);
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

void ABasePlayerCharacter::SetMeshes(bool bIsMale, FName RowName)
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

}

void ABasePlayerCharacter::Server_SetCharacterState_Implementation(ECharacterState NewState)
{
	Multicast_SetCharacterState(NewState);
	SetCharacterState(NewState);
}

void ABasePlayerCharacter::Multicast_SetCharacterState_Implementation(ECharacterState NewState)
{
	if (!HasAuthority())
	{
		SetCharacterState(NewState);
	}
}

void ABasePlayerCharacter::SetCharacterState(ECharacterState NewState)
{
	CharacterState = NewState;

	switch (CharacterState)
	{

	case ECharacterState::ECS_Unnoccupied:
		HandleStateUnnoccupied();
		break;

	case ECharacterState::ECS_Dodging:
		HandleStateDodging();
		break;

	case ECharacterState::ECS_Sprinting:
		HandleStateSprinting();
		break;

	case ECharacterState::ECS_Elimmed:
		HandleStateElimmed();
		break;
	}
}

void ABasePlayerCharacter::HandleStateUnnoccupied()
{
	if (!GetCharacterMovement()) return;

	GetCharacterMovement()->MaxWalkSpeed = 500.f;	//Sets the maximum run speed
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character doesn't move in the direction of input...
}

void ABasePlayerCharacter::HandleStateDodging()
{
	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Purple, FString("Dodge()"));
}

void ABasePlayerCharacter::HandleStateSprinting()
{
	if (!GetCharacterMovement()) return;

	//Sets the maximum run speed
	GetCharacterMovement()->MaxWalkSpeed = 800.f;
}

void ABasePlayerCharacter::HandleStateTargeting()
{
	if (!GetCharacterMovement()) return;

	GetCharacterMovement()->bOrientRotationToMovement = false; // Character doesn't move in the direction of input...
	GetCharacterMovement()->MaxWalkSpeed = 500;	//Sets the maximum run speed
}

void ABasePlayerCharacter::HandleStateElimmed()
{

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

		bIsTargeting = true;
		if (!GetCharacterMovement()) return;

		GetCharacterMovement()->bOrientRotationToMovement = false; // Character doesn't move in the direction of input...
		GetCharacterMovement()->MaxWalkSpeed = 500;	//Sets the maximum run speed
	}
}

void ABasePlayerCharacter::TargetUnlocked(UTargetingHelperComponent* UnlockedTarget, FName Socket)
{
	bIsTargeting = false;
	if (!GetCharacterMovement()) return;

	GetCharacterMovement()->MaxWalkSpeed = 500.f;	//Sets the maximum run speed
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character doesn't move in the direction of input...
}

void ABasePlayerCharacter::TargetNotFound()
{

}

void ABasePlayerCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if (!Attributes) return;
	Attributes->Server_UpdateHealth(Damage, InstigatedBy);
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
	GetMesh()->AddImpulse(FVector(100.f), FName("pelvis"), true);
	LockOnTarget->ClearTargetManual();
	TargetAttractor->bCanBeCaptured = false;

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