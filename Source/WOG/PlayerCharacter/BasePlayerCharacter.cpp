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

	//Create the modular meshes
	GetMesh()->bHiddenInGame = true;

	MainMesh = CreateDefaultSubobject< USkeletalMeshComponent>(TEXT("MainMesh"));
	MainMesh->SetupAttachment(GetRootComponent());
	MainMesh->bHiddenInGame = true;

	Head = CreateDefaultSubobject< USkeletalMeshComponent>(TEXT("Head"));
	Head->SetupAttachment(MainMesh);
	Torso = CreateDefaultSubobject< USkeletalMeshComponent>(TEXT("Torso"));
	Torso->SetupAttachment(MainMesh);
	Hips = CreateDefaultSubobject< USkeletalMeshComponent>(TEXT("Hips"));
	Hips->SetupAttachment(MainMesh);
	ArmUpperLeft = CreateDefaultSubobject< USkeletalMeshComponent>(TEXT("ArmUpperLeft"));
	ArmUpperLeft->SetupAttachment(MainMesh);
	ArmUpperRight = CreateDefaultSubobject< USkeletalMeshComponent>(TEXT("ArmUpperRight"));
	ArmUpperRight->SetupAttachment(MainMesh);
	ArmLowerLeft = CreateDefaultSubobject< USkeletalMeshComponent>(TEXT("ArmLowerLeft"));
	ArmLowerLeft->SetupAttachment(MainMesh);
	ArmLowerRight = CreateDefaultSubobject< USkeletalMeshComponent>(TEXT("ArmLowerRight"));
	ArmLowerRight->SetupAttachment(MainMesh);
	HandLeft = CreateDefaultSubobject< USkeletalMeshComponent>(TEXT("HandLeft"));
	HandLeft->SetupAttachment(MainMesh);
	HandRight = CreateDefaultSubobject< USkeletalMeshComponent>(TEXT("HandRight"));
	HandRight->SetupAttachment(MainMesh);
	LegLeft = CreateDefaultSubobject< USkeletalMeshComponent>(TEXT("LegLeft"));
	LegLeft->SetupAttachment(MainMesh);
	LegRight = CreateDefaultSubobject< USkeletalMeshComponent>(TEXT("LegRight"));
	LegRight->SetupAttachment(MainMesh);
	Hair = CreateDefaultSubobject< USkeletalMeshComponent>(TEXT("Hair"));
	Hair->SetupAttachment(MainMesh);
	Beard = CreateDefaultSubobject< USkeletalMeshComponent>(TEXT("Beard"));
	Beard->SetupAttachment(MainMesh);
	Ears = CreateDefaultSubobject< USkeletalMeshComponent>(TEXT("Ears"));
	Ears->SetupAttachment(MainMesh);
	Eyebrows = CreateDefaultSubobject< USkeletalMeshComponent>(TEXT("Eyebrows"));
	Eyebrows->SetupAttachment(MainMesh);
	Helmet = CreateDefaultSubobject< USkeletalMeshComponent>(TEXT("Helmet"));
	Helmet->SetupAttachment(MainMesh);

	CharacterState = ECharacterState::ECS_Unnoccupied;

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
	}
}

void ABasePlayerCharacter::Move(const FInputActionValue& Value)
{
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

	if (CharacterState == ECharacterState::ECS_Targeting)
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
	if (CharacterState != ECharacterState::ECS_Targeting)
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
	if (!LockOnTarget) return;
	LockOnTarget->EnableTargeting();
}

void ABasePlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABasePlayerCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("%s Damaged by: %s"),
		*GetName(), *DamageCauser->GetName()));
	if (!Attributes) return;
	Attributes->Server_UpdateHealth(Damage, InstigatedBy);
}

void ABasePlayerCharacter::Elim(bool bPlayerLeftGame)
{
	Multicast_Elim(bPlayerLeftGame);
}

void ABasePlayerCharacter::Multicast_Elim_Implementation(bool bPlayerLeftGame)
{
	//GetCharacterMovement()->DisableMovement();
	Jump();
	GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Purple, FString("Multicast_Elim()"));
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

	MainMesh->SetSkinnedAssetAndUpdate(MeshRow->BaseMesh, true);
	MainMesh->SetAnimInstanceClass(MeshRow->AnimBP);

	Head->SetSkinnedAssetAndUpdate(MeshRow->Head, true);
	Head->SetMaterial(0, CharacterMI);

	Torso->SetSkinnedAssetAndUpdate(MeshRow->Torso, true);
	Torso->SetMaterial(0, CharacterMI);

	Hips->SetSkinnedAssetAndUpdate(MeshRow->Hips, true);
	Hips->SetMaterial(0, CharacterMI);

	ArmUpperLeft->SetSkinnedAssetAndUpdate(MeshRow->ArmUpperLeft, true);
	ArmUpperLeft->SetMaterial(0, CharacterMI);

	ArmUpperRight->SetSkinnedAssetAndUpdate(MeshRow->ArmUpperRight, true);
	ArmUpperRight->SetMaterial(0, CharacterMI);

	ArmLowerLeft->SetSkinnedAssetAndUpdate(MeshRow->ArmLowerLeft, true);
	ArmLowerLeft->SetMaterial(0, CharacterMI);

	ArmLowerRight->SetSkinnedAssetAndUpdate(MeshRow->ArmLowerRight, true);
	ArmLowerRight->SetMaterial(0, CharacterMI);

	HandLeft->SetSkinnedAssetAndUpdate(MeshRow->HandLeft, true);
	HandLeft->SetMaterial(0, CharacterMI);

	HandRight->SetSkinnedAssetAndUpdate(MeshRow->HandRight, true);
	HandRight->SetMaterial(0, CharacterMI);

	LegLeft->SetSkinnedAssetAndUpdate(MeshRow->LegLeft, true);
	LegLeft->SetMaterial(0, CharacterMI);

	LegRight->SetSkinnedAssetAndUpdate(MeshRow->LegRight, true);
	LegRight->SetMaterial(0, CharacterMI);

	Hair->SetSkinnedAssetAndUpdate(MeshRow->Hair, true);
	Hair->SetMaterial(0, CharacterMI);

	Beard->SetSkinnedAssetAndUpdate(MeshRow->Beard, true);
	Beard->SetMaterial(0, CharacterMI);

	Ears->SetSkinnedAssetAndUpdate(MeshRow->Ears, true);
	Ears->SetMaterial(0, CharacterMI);

	Eyebrows->SetSkinnedAssetAndUpdate(MeshRow->Eyebrows, true);
	Eyebrows->SetMaterial(0, CharacterMI);

	Helmet->SetSkinnedAssetAndUpdate(MeshRow->Helmet, true);
	Helmet->SetMaterial(0, CharacterMI);

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

	case ECharacterState::ECS_Targeting:
		HandleStateTargeting();
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
		if (CharacterState != ECharacterState::ECS_Targeting)
		{
			Server_SetCharacterState(ECharacterState::ECS_Targeting);
		}
	}
}

void ABasePlayerCharacter::TargetUnlocked(UTargetingHelperComponent* UnlockedTarget, FName Socket)
{
	Server_SetCharacterState(ECharacterState::ECS_Unnoccupied);
	GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Red, FString::Printf(TEXT("Target unlocked")));
}

void ABasePlayerCharacter::TargetNotFound()
{
	GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Red, FString::Printf(TEXT("Target not found")));
}
