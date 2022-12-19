// Fill out your copyright notice in the Description page of Project Settings.


#include "BasePlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"


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

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rate for input
	TurnRateGamepad = 50.f;

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

	Head = CreateDefaultSubobject< USkeletalMeshComponent>(TEXT("Head"));
	Head->SetupAttachment(GetMesh());
	Torso = CreateDefaultSubobject< USkeletalMeshComponent>(TEXT("Torso"));
	Torso->SetupAttachment(GetMesh());
	Hips = CreateDefaultSubobject< USkeletalMeshComponent>(TEXT("Hips"));
	Hips->SetupAttachment(GetMesh());
	ArmUpperLeft = CreateDefaultSubobject< USkeletalMeshComponent>(TEXT("ArmUpperLeft"));
	ArmUpperLeft->SetupAttachment(GetMesh());
	ArmUpperRight = CreateDefaultSubobject< USkeletalMeshComponent>(TEXT("ArmUpperRight"));
	ArmUpperRight->SetupAttachment(GetMesh());
	ArmLowerLeft = CreateDefaultSubobject< USkeletalMeshComponent>(TEXT("ArmLowerLeft"));
	ArmLowerLeft->SetupAttachment(GetMesh());
	ArmLowerRight = CreateDefaultSubobject< USkeletalMeshComponent>(TEXT("ArmLowerRight"));
	ArmLowerRight->SetupAttachment(GetMesh());
	HandLeft = CreateDefaultSubobject< USkeletalMeshComponent>(TEXT("HandLeft"));
	HandLeft->SetupAttachment(GetMesh());
	HandRight = CreateDefaultSubobject< USkeletalMeshComponent>(TEXT("HandRight"));
	HandRight->SetupAttachment(GetMesh());
	LegLeft = CreateDefaultSubobject< USkeletalMeshComponent>(TEXT("LegLeft"));
	LegLeft->SetupAttachment(GetMesh());
	LegRight = CreateDefaultSubobject< USkeletalMeshComponent>(TEXT("LegRight"));
	LegRight->SetupAttachment(GetMesh());
	Hair = CreateDefaultSubobject< USkeletalMeshComponent>(TEXT("Hair"));
	Hair->SetupAttachment(GetMesh());
	Beard = CreateDefaultSubobject< USkeletalMeshComponent>(TEXT("Beard"));
	Beard->SetupAttachment(GetMesh());
	Ears = CreateDefaultSubobject< USkeletalMeshComponent>(TEXT("Ears"));
	Ears->SetupAttachment(GetMesh());
	Eyebrows = CreateDefaultSubobject< USkeletalMeshComponent>(TEXT("Eyebrows"));
	Eyebrows->SetupAttachment(GetMesh());
	Helmet = CreateDefaultSubobject< USkeletalMeshComponent>(TEXT("Helmet"));
	Helmet->SetupAttachment(GetMesh());
}

// Called to bind functionality to input
void ABasePlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("Move Forward / Backward", this, &ThisClass::MoveForward);
	PlayerInputComponent->BindAxis("Move Right / Left", this, &ThisClass::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn Right / Left Mouse", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("Turn Right / Left Gamepad", this, &ThisClass::TurnAtRate);
	PlayerInputComponent->BindAxis("Look Up / Down Mouse", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Look Up / Down Gamepad", this, &ThisClass::LookUpAtRate);
}

void ABasePlayerCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void ABasePlayerCharacter::MoveRight(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

void ABasePlayerCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

void ABasePlayerCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

void ABasePlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABasePlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}