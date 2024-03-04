// Fill out your copyright notice in the Description page of Project Settings.

#include "Enemies/WOGRaven.h"
#include "WOG.h"
#include "PlayerController/WOGPlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Net/UnrealNetwork.h"
#include "Utilities/WOGRavenMarker.h"
#include "Subsystems/WOGWorldSubsystem.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Data/WOGGameplayTags.h"
#include "Subsystems/WOGUIManagerSubsystem.h"


AWOGRaven::AWOGRaven()
{
	PrimaryActorTick.bCanEverTick = false;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->DefaultLandMovementMode = EMovementMode::MOVE_Flying;
	GetCharacterMovement()->MaxFlySpeed = 700.f;
	GetCharacterMovement()->BrakingDecelerationFlying = 1500.f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 750.0f; // The camera follows at this distance behind the character
	CameraBoom->SocketOffset = FVector(0.f, -100.f, 100.f);
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->bEnableCameraRotationLag = true;
	CameraBoom->CameraLagSpeed = 7.5f;
	CameraBoom->CameraRotationLagSpeed = 5.f;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
}

void AWOGRaven::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWOGRaven, SpawnedMarkers);
}

void AWOGRaven::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->ClearAllMappings();
			Subsystem->AddMappingContext(RavenMappingContext, 0);
		}

		if (PlayerController->PlayerCameraManager)
		{
			PlayerController->PlayerCameraManager->ViewPitchMin = -60.f;
			PlayerController->PlayerCameraManager->ViewPitchMax = 60.f;
		}	
	}

	TObjectPtr<UWOGWorldSubsystem> WorldSubsystem = GetWorld()->GetSubsystem<UWOGWorldSubsystem>();
	if (WorldSubsystem)
	{
		WorldSubsystem->TimeOfDayChangedDelegate.AddDynamic(this, &ThisClass::TODChanged);
		WorldSubsystem->OnKeyTimeHitDelegate.AddDynamic(this, &ThisClass::KeyTimeHit);
	}

	GiveDefaultAbilities();
	ApplyDefaultEffects();
}

void AWOGRaven::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		//Move:
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::MoveActionPressed);
		//Look:
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ThisClass::LookActionPressed);
		//Place marker
		EnhancedInputComponent->BindAction(PrimaryLightAction, ETriggerEvent::Triggered, this, &ThisClass::PrimaryActionTriggered);
		//Remove marker
		EnhancedInputComponent->BindAction(SecondaryAction, ETriggerEvent::Triggered, this, &ThisClass::SecondaryActionTriggered);

	}
}

void AWOGRaven::MoveActionPressed(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();
	if (Controller != nullptr && MovementVector.Y > 0.f)
	{
		// add movement 
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
	}
}

void AWOGRaven::LookActionPressed(const FInputActionValue& Value)
{
	FVector2D LookVector = Value.Get<FVector2D>();
	if (Controller != nullptr)
	{
		AddControllerYawInput(FMath::Clamp(LookVector.X, -1.f, 1.f));
		AddControllerPitchInput(FMath::Clamp(LookVector.Y, -1.f, 1.f));
	}
}

void AWOGRaven::PrimaryActionTriggered(const FInputActionValue& Value)
{
	if (SpawnedMarkers.Num() > 2)
	{
		if (OwnerPC)
		{
			TObjectPtr<UWOGUIManagerSubsystem> UIManager = ULocalPlayer::GetSubsystem<UWOGUIManagerSubsystem>(OwnerPC->GetLocalPlayer());
			if (UIManager)
			{
				UIManager->CreateGenericWarningWidget(FString("TooManyMarkers"));
			}
		}

		UE_LOG(WOGLogSpawn, Error, TEXT("Too many markers placed"));
		return;
	}

	FHitResult HitResult;
	FVector Start = FollowCamera->GetComponentLocation() + (FollowCamera->GetForwardVector() * 750.f);
	FVector End = FollowCamera->GetComponentLocation() + (FollowCamera->GetForwardVector() * 500000.f);

	GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECollisionChannel::ECC_Visibility);

	if (HitResult.bBlockingHit)
	{
		Server_SpawnMarker(HitResult.ImpactPoint);
	}
}

void AWOGRaven::SecondaryActionTriggered(const FInputActionValue& Value)
{
	Server_DestroyMarker();
}

void AWOGRaven::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (IsPlayerControlled())
	{
		SetActorRotation(FRotator());
		bUseControllerRotationPitch = true;
		bUseControllerRotationYaw = true;
		GetCharacterMovement()->MaxFlySpeed = 1200.f;
	}
	else
	{
		bUseControllerRotationPitch = false;
		bUseControllerRotationYaw = false;
		GetCharacterMovement()->MaxFlySpeed = 700.f;
	}
}

void AWOGRaven::UnpossessMinion_Implementation()
{
	FGameplayEventData EventPayload;
	EventPayload.EventTag = TAG_Event_Spawn_Unpossess;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, TAG_Event_Spawn_Unpossess, EventPayload);

	TObjectPtr<AWOGPlayerController> PlayerController = Cast<AWOGPlayerController>(GetController());
	if (!PlayerController) return;

	TObjectPtr<UWOGUIManagerSubsystem> UIManager = ULocalPlayer::GetSubsystem<UWOGUIManagerSubsystem>(PlayerController->GetLocalPlayer());
	if (UIManager)
	{
		UIManager->RemoveRavenMarkerWidget();
	}
}

void AWOGRaven::TODChanged(ETimeOfDay TOD)
{
	switch (TOD)
	{
	case ETimeOfDay::TOD_Dusk1:
		Client_RemoveRavenMarkerWidget();
		Server_UnpossessMinion();
		break;
	case ETimeOfDay::TOD_Dusk2:
		Client_RemoveRavenMarkerWidget();
		Server_UnpossessMinion();
		break;
	case ETimeOfDay::TOD_Dusk3:
		Client_RemoveRavenMarkerWidget();
		Server_UnpossessMinion();
		break;
	}
}

void AWOGRaven::KeyTimeHit(int32 CurrentTime)
{
	if (CurrentTime == 350 && IsLocallyControlled())
	{
		Client_RemoveRavenMarkerWidget();
		Server_UnpossessMinion();
	}
}

void AWOGRaven::Client_RemoveRavenMarkerWidget_Implementation()
{
	TObjectPtr<AWOGPlayerController> PlayerController = Cast<AWOGPlayerController>(GetController());
	if (!PlayerController) return;

	TObjectPtr<UWOGUIManagerSubsystem> UIManager = ULocalPlayer::GetSubsystem<UWOGUIManagerSubsystem>(PlayerController->GetLocalPlayer());
	if (UIManager)
	{
		UIManager->RemoveRavenMarkerWidget();
	}
}

void AWOGRaven::Server_SpawnMarker_Implementation(const FVector_NetQuantize& SpawnLocation)
{
	if (!IsValid(MarkerClass))
	{
		UE_LOG(WOGLogSpawn, Error, TEXT("No valid RavenMarker class"));
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;

	FTransform Transform;
	Transform.SetLocation(SpawnLocation);
	Transform.SetRotation(FRotator(0.f, 0.f, 0.f).Quaternion());
	Transform.SetScale3D(FVector(4.f));

	TObjectPtr<AWOGRavenMarker> Marker = GetWorld()->SpawnActor<AWOGRavenMarker>(MarkerClass, Transform, SpawnParams);
	if (Marker)
	{

		SpawnedMarkers.AddUnique(Marker);
	}
}

void AWOGRaven::Server_DestroyMarker_Implementation()
{
	if (SpawnedMarkers.IsEmpty())
	{
		UE_LOG(WOGLogSpawn, Error, TEXT("Marker array Empty"));
		return;
	}

	TObjectPtr<AWOGRavenMarker> TempMarker = SpawnedMarkers[SpawnedMarkers.Num() - 1];
	if (!TempMarker) return;

	TempMarker->Destroy();
	SpawnedMarkers.Remove(TempMarker);
}

void AWOGRaven::Server_UnpossessMinion_Implementation()
{
	if (IsPlayerControlled())
	{
		TObjectPtr<AWOGPlayerController> PlayerController = Cast<AWOGPlayerController>(GetController());
		if (PlayerController)
		{
			PlayerController->Server_UnpossessMinion(this);
		}
	}
}
