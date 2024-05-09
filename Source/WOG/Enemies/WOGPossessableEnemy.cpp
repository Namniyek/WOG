// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGPossessableEnemy.h"
#include "WOG.h"
#include "WOG/PlayerController/WOGPlayerController.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "AbilitySystemComponent.h"
#include "Data/WOGGameplayTags.h"
#include "Subsystems/WOGWorldSubsystem.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "TargetSystemComponent.h"
#include "Subsystems/WOGUIManagerSubsystem.h"
#include "ActorComponents/WOGAbilitySystemComponent.h"

AWOGPossessableEnemy::AWOGPossessableEnemy()
{
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

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

	TargetComponent = CreateDefaultSubobject<UTargetSystemComponent>(TEXT("TargetComponent"));
	if (TargetComponent)
	{
		TargetComponent->OnTargetLockedOn.AddDynamic(this, &ThisClass::TargetLocked);
		TargetComponent->OnTargetLockedOff.AddDynamic(this, &ThisClass::TargetUnlocked);
	}
}

void AWOGPossessableEnemy::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWOGPossessableEnemy, CurrentTarget);
}

void AWOGPossessableEnemy::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	SetOwner(NewController);
	ToggleStrafeMovement(true);
}

void AWOGPossessableEnemy::BeginPlay()
{
	Super::BeginPlay();

	TObjectPtr<UWOGWorldSubsystem> WorldSubsystem = GetWorld()->GetSubsystem<UWOGWorldSubsystem>();
	if (WorldSubsystem)
	{
		WorldSubsystem->OnKeyTimeHitDelegate.AddDynamic(this, &ThisClass::KeyTimeHit);
	}
}

void AWOGPossessableEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	UE_LOG(WOGLogSpawn, Display, TEXT("SetupPlayerInputComponent() called"));

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		//Move:
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::MoveActionPressed);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &ThisClass::MoveActionReleased);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Canceled, this, &ThisClass::MoveActionReleased);
		//Look:
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ThisClass::LookActionPressed);
		//Jump:
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ThisClass::JumpActionPressed);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		//Sprint:
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Triggered, this, &ThisClass::SprintActionPressed);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ThisClass::StopSprinting);
		//Unpossess:
		EnhancedInputComponent->BindAction(UnpossessAction, ETriggerEvent::Triggered, this, &ThisClass::UnpossessActionPressed);
		//Target
		EnhancedInputComponent->BindAction(TargetAction, ETriggerEvent::Completed, this, &ThisClass::TargetActionPressed);
		EnhancedInputComponent->BindAction(CycleTargetAction, ETriggerEvent::Triggered, this, &ThisClass::CycleTargetActionPressed);
		//Attacks
		EnhancedInputComponent->BindAction(PrimaryAttackAction, ETriggerEvent::Triggered, this, &ThisClass::PrimaryAttackActionPressed);
		EnhancedInputComponent->BindAction(MainAltAttackAction, ETriggerEvent::Triggered, this, &ThisClass::MainAltAttackActionPressed);
		EnhancedInputComponent->BindAction(SecondaryAltAttackAction, ETriggerEvent::Triggered, this, &ThisClass::SecondaryAltAttackActionPressed);
		EnhancedInputComponent->BindAction(RangedAttackAction, ETriggerEvent::Triggered, this, &ThisClass::RangedAttackActionPressed);
		EnhancedInputComponent->BindAction(CloseAttackAction, ETriggerEvent::Triggered, this, &ThisClass::CloseAttackActionPressed);
		EnhancedInputComponent->BindAction(BlockAction, ETriggerEvent::Started, this, &ThisClass::BlockActionPressed);
		EnhancedInputComponent->BindAction(BlockAction, ETriggerEvent::Triggered, this, &ThisClass::BlockActionReleased);
		//Dodge
		EnhancedInputComponent->BindAction(DodgeAction, ETriggerEvent::Triggered, this, &ThisClass::DodgeActionPressed);
	}
}

void AWOGPossessableEnemy::MoveActionPressed(const FInputActionValue& Value)
{
	if (HasMatchingGameplayTag(TAG_State_Dead)) return;
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
		
		if (!bIsTargeting)
		{
			ToggleStrafeMovement(false);
		}
	}
}

void AWOGPossessableEnemy::MoveActionReleased()
{
	if (!bIsTargeting)
	{
		ToggleStrafeMovement(true);
	}
}

void AWOGPossessableEnemy::LookActionPressed(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AWOGPossessableEnemy::JumpActionPressed(const FInputActionValue& Value)
{
	SendAbilityLocalInput(EWOGAbilityInputID::Jump);
}

void AWOGPossessableEnemy::PrimaryAttackActionPressed(const FInputActionValue& Value)
{
	SendAbilityLocalInput(EWOGAbilityInputID::AttackLight);
	UE_LOG(WOGLogSpawn, Display, TEXT("Primary Light button pressed"));
}

void AWOGPossessableEnemy::MainAltAttackActionPressed(const FInputActionValue& Value)
{
	SendAbilityLocalInput(EWOGAbilityInputID::AttackAltMain);
	UE_LOG(WOGLogSpawn, Display, TEXT("Main Alt Attack button pressed"));
}

void AWOGPossessableEnemy::SecondaryAltAttackActionPressed(const FInputActionValue& Value)
{
	SendAbilityLocalInput(EWOGAbilityInputID::AttackAltSec);
	UE_LOG(WOGLogSpawn, Display, TEXT("Secondary Alt Attack button pressed"));
}

void AWOGPossessableEnemy::RangedAttackActionPressed(const FInputActionValue& Value)
{
	SendAbilityLocalInput(EWOGAbilityInputID::Ranged);
	UE_LOG(WOGLogSpawn, Display, TEXT("Ranged Attack button pressed"));
}

void AWOGPossessableEnemy::CloseAttackActionPressed(const FInputActionValue& Value)
{
	SendAbilityLocalInput(EWOGAbilityInputID::AttackClose);
	UE_LOG(WOGLogSpawn, Display, TEXT("Close Attack button pressed"));
}

void AWOGPossessableEnemy::BlockActionPressed(const FInputActionValue& Value)
{
	SendAbilityLocalInput(EWOGAbilityInputID::Block);
	UE_LOG(WOGLogSpawn, Display, TEXT("Block button pressed"));
	bSecondaryButtonPressed = true;
}

void AWOGPossessableEnemy::BlockActionReleased(const FInputActionValue& Value)
{
	FGameplayEventData EventPayload;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, TAG_Event_Weapon_Block_Stop, EventPayload);
	bSecondaryButtonPressed = false;
}

void AWOGPossessableEnemy::UnpossessActionPressed(const FInputActionValue& Value)
{
	TObjectPtr<UWOGUIManagerSubsystem> UIManager = ULocalPlayer::GetSubsystem<UWOGUIManagerSubsystem>(OwnerPC->GetLocalPlayer());
	if (UIManager)
	{
		UIManager->RemoveCrosshairWidget();
	}
	Server_UnpossessMinion();
}

void AWOGPossessableEnemy::SprintActionPressed(const FInputActionValue& Value)
{
	SendAbilityLocalInput(EWOGAbilityInputID::Sprint);
	UE_LOG(WOGLogSpawn, Display, TEXT("Sprint button pressed"));
}

void AWOGPossessableEnemy::StopSprinting()
{
	FGameplayEventData EventPayload;
	EventPayload.EventTag = TAG_Event_Movement_Sprint_Stop;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, TAG_Event_Movement_Sprint_Stop, EventPayload);
}

void AWOGPossessableEnemy::TargetActionPressed(const FInputActionValue& Value)
{
	if (HasMatchingGameplayTag(TAG_State_Dead)) return;
	if (HasMatchingGameplayTag(TAG_State_Dodging)) return;
	if (!TargetComponent) return;

	TargetComponent->TargetActor();
}

void AWOGPossessableEnemy::CycleTargetActionPressed(const FInputActionValue& Value)
{
	if (HasMatchingGameplayTag(TAG_State_Dead)) return;

	if (!TargetComponent) return;
	float CycleFloat = Value.Get<float>();

	TargetComponent->TargetActorWithAxisInput(CycleFloat);
}

void AWOGPossessableEnemy::DodgeActionPressed(const FInputActionValue& Value)
{
	SendAbilityLocalInput(EWOGAbilityInputID::Dodge);
	UE_LOG(WOGLogSpawn, Display, TEXT("Dodge button pressed"));
}

void AWOGPossessableEnemy::Elim(bool bPlayerLeftGame)
{
	UE_LOG(WOGLogCombat, Display, TEXT("Elim() called from PossessableEnemy"))
	Server_UnpossessMinion();
	Super::Elim(bPlayerLeftGame);
}

void AWOGPossessableEnemy::UnpossessMinion_Implementation()
{
	Server_UnpossessMinion();
}

bool AWOGPossessableEnemy::CanBePossessed_Implementation() const
{
	return true;
}

void AWOGPossessableEnemy::HandleTODChange()
{
	switch (CurrentTOD)
	{
	case ETimeOfDay::TOD_Dawn2:
		Destroy();
		break;
	case ETimeOfDay::TOD_Dawn3:
		Destroy();
		break;
	case ETimeOfDay::TOD_Dawn4:
		Destroy();
		break;
	default:
		break;
	}
}

void AWOGPossessableEnemy::KeyTimeHit(int32 CurrentTime)
{
	if (CurrentTime == 350)
	{
		Server_UnpossessMinion();
	}
	if (CurrentTime == 1040)
	{
		Server_UnpossessMinion();
	}
}

void AWOGPossessableEnemy::Server_UnpossessMinion_Implementation()
{
	if (IsPlayerControlled())
	{
		const TObjectPtr<AWOGPlayerController> PlayerController = Cast<AWOGPlayerController>(GetController());
		if (PlayerController)
		{
			PlayerController->Server_UnpossessMinion(this);
		}	

		OwnerPC = nullptr;

		SpawnDefaultController();
	}

}

void AWOGPossessableEnemy::TargetLocked(AActor* NewTarget)
{
	if (!NewTarget)
	{
		GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Red, FString::Printf(TEXT("Target invalid")));
		return;
	}

	bIsTargeting = true;
	Server_SetCurrentTarget(NewTarget);

	ToggleStrafeMovement(true);
	ISpawnInterface::Execute_SetMovementSpeed(this, ECharacterMovementSpeed::ECMS_Walking);

	TObjectPtr<UWOGUIManagerSubsystem> UIManager = ULocalPlayer::GetSubsystem<UWOGUIManagerSubsystem>(OwnerPC->GetLocalPlayer());
	if (UIManager)
	{
		UIManager->RemoveCrosshairWidget();
	}
}

void AWOGPossessableEnemy::TargetUnlocked(AActor* OldTarget)
{
	bIsTargeting = false;
	Server_SetCurrentTarget();

	ToggleStrafeMovement(false);
	ISpawnInterface::Execute_SetMovementSpeed(this, ECharacterMovementSpeed::ECMS_Running);

	TObjectPtr<UWOGUIManagerSubsystem> UIManager = ULocalPlayer::GetSubsystem<UWOGUIManagerSubsystem>(OwnerPC->GetLocalPlayer());
	if (UIManager)
	{
		UIManager->AddCrosshairWidget();
	}
}

void AWOGPossessableEnemy::ToggleStrafeMovement(bool bIsStrafe)
{
	if (!AnimManager) return;
	if (bIsStrafe)
	{
		//Start strafe movement
		AnimManager->SetupRotation(EAGR_RotationMethod::DesiredAtAngle, 150.f, 75.f, 5.f);
	}
	else
	{
		if (UKismetMathLibrary::NearlyEqual_FloatFloat(GetCharacterMovement()->Velocity.Length(), 0, 5.f)) //character idle
		{
			//Stop strafe movement while idle
			AnimManager->SetupRotation(EAGR_RotationMethod::DesiredAtAngle, 150.f, 75.f, 5.f);
		}
		else //character moving
		{
			//Stop strafe movement while moving
			AnimManager->SetupRotation(EAGR_RotationMethod::RotateToVelocity, 300.f, 75.f, 5.f);
		}
	}
}

void AWOGPossessableEnemy::Server_SetCurrentTarget_Implementation(AActor* NewTarget)
{
	CurrentTarget = NewTarget;

	if (CurrentTarget)
	{
		ISpawnInterface::Execute_SetMovementSpeed(this, ECharacterMovementSpeed::ECMS_Walking);
	}
	else
	{
		ISpawnInterface::Execute_SetMovementSpeed(this, ECharacterMovementSpeed::ECMS_Running);
	}
}
