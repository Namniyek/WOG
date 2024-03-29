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
#include "WOG/GameMode/WOGGameMode.h"
#include "WOG/PlayerController/WOGPlayerController.h"
#include "WOG/Weapons/WOGBaseWeapon.h"
#include "WOG/ActorComponents/WOGCombatComponent.h"
#include "Kismet/GameplayStatics.h"
#include "WOG/AnimInstance/WOGBaseAnimInstance.h"
#include "Sound/SoundCue.h"
#include "WOG/ActorComponents/WOGAbilitiesComponent.h"


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
	Combat = CreateDefaultSubobject<UWOGCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);
	Abilities = CreateDefaultSubobject<UWOGAbilitiesComponent>(TEXT("AbilitiesComponent"));
	Abilities->SetIsReplicated(true);


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
	if (CharacterState == ECharacterState::ECS_Elimmed) return;
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
	if (CharacterState == ECharacterState::ECS_Attacking) return;
	if (CharacterState == ECharacterState::ECS_Staggered) return;

	ACharacter::Jump();
}

void ABasePlayerCharacter::DodgeActionPressed(const FInputActionValue& Value)
{
	if (CharacterState == ECharacterState::ECS_Unnoccupied)
	{
		Server_SetCharacterState(ECharacterState::ECS_Dodging);
	}
	else
	{
		return;
	}

}

void ABasePlayerCharacter::StopDodging()
{
	if (CharacterState == ECharacterState::ECS_Dodging)
	{
		Server_SetCharacterState(ECharacterState::ECS_Unnoccupied);
	}
}

void ABasePlayerCharacter::Dodge()
{
	TObjectPtr<UWOGBaseAnimInstance> CharacterAnimInstance = Cast< UWOGBaseAnimInstance>(GetMesh()->GetAnimInstance());
	if (!CharacterAnimInstance) return;

	//GetCharacterMovement()->MaxWalkSpeed = 1000.f;	//Sets the maximum run speed

	if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetDodgeMontage())
	{
		CharacterAnimInstance->Montage_Play(Combat->EquippedWeapon->GetDodgeMontage(), 1.f);
		CharacterAnimInstance->Montage_JumpToSection(CharacterAnimInstance->GetMovementDirection());
	}

	else if (DodgeMontage)
	{
		CharacterAnimInstance->Montage_Play(DodgeMontage, 1.f);
		CharacterAnimInstance->Montage_JumpToSection(CharacterAnimInstance->GetMovementDirection());
	}
}

void ABasePlayerCharacter::SprintActionPressed()
{
	if (CharacterState != ECharacterState::ECS_Unnoccupied) return;
	if (!bIsTargeting)
	{
		Server_SetCharacterState(ECharacterState::ECS_Sprinting);
	}
}

void ABasePlayerCharacter::StopSprinting()
{
	Server_SetCharacterState(ECharacterState::ECS_Unnoccupied);
}

void ABasePlayerCharacter::TargetActionPressed(const FInputActionValue& Value)
{
	if (CharacterState == ECharacterState::ECS_Elimmed) return;
	if (CharacterState == ECharacterState::ECS_Staggered) return;

	if (!LockOnTarget) return;
	LockOnTarget->EnableTargeting();
}

void ABasePlayerCharacter::CycleTargetActionPressed(const FInputActionValue& Value)
{
	if (CharacterState == ECharacterState::ECS_Elimmed) return;

	if (!LockOnTarget) return;
	float CycleFloat = Value.Get<float>();
	FVector2D CycleVector = FVector2D(CycleFloat, 0.f);

	LockOnTarget->SwitchTargetManual(CycleVector);
}

void ABasePlayerCharacter::AbilitiesButtonPressed(const FInputActionValue& Value)
{
	//TO BE OVERRIDEN IN CHILDREN
}

void ABasePlayerCharacter::PrimaryLightButtonPressed(const FInputActionValue& Value)
{
	if (CharacterState == ECharacterState::ECS_Elimmed) return;
	if (CharacterState == ECharacterState::ECS_Staggered) return;
	if (CharacterState == ECharacterState::ECS_Dodging) return;

	if (!Combat || !Abilities) return;
	if (Combat->EquippedWeapon)
	{
		Combat->AttackLight();
	}
	if (Abilities->EquippedAbility)
	{
		Abilities->UseAbilityActionPressed();
	}
}

void ABasePlayerCharacter::PrimaryArmHeavyAttack(FInputActionValue ActionValue, float ElapsedTime, float TriggeredTime)
{
	if (CharacterState == ECharacterState::ECS_Elimmed) return;
	if (CharacterState == ECharacterState::ECS_Staggered) return;
	if (CharacterState == ECharacterState::ECS_Dodging) return;

	if (!Combat || !Combat->GetEquippedWeapon()) return;
	
	if (ElapsedTime > 0.2f && !Combat->GetEquippedWeapon()->GetIsArmingHeavy())
	{
		Combat->AttackHeavyArm();
		UE_LOG(LogTemp, Warning, TEXT("ArmingHeavyAttack"));
	}
}

void ABasePlayerCharacter::PrimaryHeavyAttackCanceled(const FInputActionValue& Value)
{
	if (CharacterState == ECharacterState::ECS_Elimmed) return;
	if (CharacterState == ECharacterState::ECS_Staggered) return;
	if (CharacterState == ECharacterState::ECS_Dodging) return;

	if (!Combat) return;
	if (Combat->EquippedWeapon)
	{
		Combat->AttackHeavyCanceled();
	}
}

void ABasePlayerCharacter::PrimaryExecuteHeavyAttack(const FInputActionValue& Value)
{
	if (CharacterState == ECharacterState::ECS_Elimmed) return;
	if (CharacterState == ECharacterState::ECS_Staggered) return;
	if (CharacterState == ECharacterState::ECS_Dodging) return;

	if (!Combat || !Combat->GetEquippedWeapon()) return;

	Combat->AttackHeavy();
}

void ABasePlayerCharacter::SecondaryButtonPressed(const FInputActionValue& Value)
{
	if (CharacterState == ECharacterState::ECS_Elimmed) return;
	if (CharacterState == ECharacterState::ECS_Staggered) return;
	if (CharacterState == ECharacterState::ECS_Dodging) return;

	if (!Combat) return;
	Combat->Block();
	GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Purple, FString("StartBlocking"));
}

void ABasePlayerCharacter::SecondaryButtonReleased(const FInputActionValue& Value)
{
	if (CharacterState == ECharacterState::ECS_Elimmed) return;
	if (CharacterState == ECharacterState::ECS_Staggered) return;
	if (CharacterState == ECharacterState::ECS_Dodging) return;

	if (!Combat) return;
	Combat->StopBlocking();
	GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Purple, FString("StopBlocking"));
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

	Combat->SetDefaultWeaponClass(MeshRow->DefaultWeapon);
	UnarmedHurtMontage = MeshRow->UnarmedHurtMontage;
	DodgeMontage = MeshRow->DodgeMontage;
}

void ABasePlayerCharacter::HandleStateUnnoccupied()
{
	if (!GetCharacterMovement()) return;

	GetCharacterMovement()->MaxWalkSpeed = 500.f;	//Sets the maximum run speed
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character doesn't move in the direction of input...
}

void ABasePlayerCharacter::HandleStateDodging()
{
	Dodge();
}

void ABasePlayerCharacter::HandleStateSprinting()
{
	if (!GetCharacterMovement()) return;

	//Sets the maximum run speed
	GetCharacterMovement()->MaxWalkSpeed = 800.f;
}

void ABasePlayerCharacter::HandleStateElimmed(AController* InstigatedBy)
{
	if (Combat->MainWeapon)
	{
		Combat->MainWeapon->Destroy();
	}
	if (Combat->SecondaryWeapon)
	{
		Combat->SecondaryWeapon->Destroy();
	}

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

void ABasePlayerCharacter::HandleStateAttacking()
{
	//TO-DO What happens when character attacks. 
}

void ABasePlayerCharacter::HandleStateStaggered()
{
	UAnimInstance* CharacterAnimInstance = GetMesh()->GetAnimInstance();
	if (!CharacterAnimInstance) return;

	if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetBlockMontage())
	{
		CharacterAnimInstance->Montage_Play(Combat->GetEquippedWeapon()->GetBlockMontage(), 1.f);
		CharacterAnimInstance->Montage_JumpToSection(FName("Knockback"));
	}
}

void ABasePlayerCharacter::BroadcastHit_Implementation(AActor* AgressorActor, const FHitResult& Hit, const float& DamageToApply, AActor* InstigatorWeapon)
{
	if (CharacterState == ECharacterState::ECS_Elimmed) return;
	if (CharacterState == ECharacterState::ECS_Dodging) return;

	if (Combat->GetEquippedWeapon() && Combat->GetEquippedWeapon()->GetWeaponState() == EWeaponState::EWS_Blocking)
	{
		if (IsHitFrontal(60.f, this, AgressorActor))
		{
			if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetCanParry())
			{
				TObjectPtr<AWOGBaseCharacter> AgressorCharacter = Cast<AWOGBaseCharacter>(AgressorActor);
				if (AgressorCharacter)
				{
					AgressorCharacter->Server_SetCharacterState(ECharacterState::ECS_Staggered);
				}
			}

			TObjectPtr<AWOGBaseWeapon> Weapon = Cast<AWOGBaseWeapon>(InstigatorWeapon);
			Multicast_HandleCosmeticHit(ECosmeticHit::ECH_BlockingWeapon, Hit, InstigatorWeapon->GetActorLocation(), Weapon);
			return;
		}
	}
	if (Combat->GetEquippedWeapon() && (Combat->GetEquippedWeapon()->GetWeaponState() == EWeaponState::EWS_AttackLight))
	{
		if (IsHitFrontal(60.f, this, AgressorActor))
		{
			TObjectPtr<AWOGBaseWeapon> Weapon = Cast<AWOGBaseWeapon>(InstigatorWeapon);
			TObjectPtr<ABasePlayerCharacter> Agressor = Cast<ABasePlayerCharacter>(AgressorActor);

			if (Agressor)
			{
				Agressor->Server_SetCharacterState(ECharacterState::ECS_Staggered);
			}
			Server_SetCharacterState(ECharacterState::ECS_Staggered);
			Multicast_HandleCosmeticHit(ECosmeticHit::ECH_AttackingWeapon, Hit, InstigatorWeapon->GetActorLocation(), Weapon);
			return;
		}
	}

	TObjectPtr<AWOGBaseWeapon> Weapon = Cast<AWOGBaseWeapon>(InstigatorWeapon);
	if (Weapon)
	{
		Multicast_HandleCosmeticHit(ECosmeticHit::ECH_BodyHit, Hit, InstigatorWeapon->GetActorLocation(), Weapon);
	}

	TObjectPtr<AWOGBaseCharacter> AgressorCharacter = Cast<AWOGBaseCharacter>(AgressorActor);
	if (AgressorCharacter)
	{
		UGameplayStatics::ApplyDamage(this, -DamageToApply, AgressorCharacter->GetController(), AgressorActor, UDamageType::StaticClass());
	}
}

void ABasePlayerCharacter::HandleCosmeticBodyHit(const FHitResult& Hit, const FVector& WeaponLocation, const AWOGBaseWeapon* InstigatorWeapon)
{
	FName HitDirection = CalculateHitDirection(Hit, InstigatorWeapon->OwnerCharacter->GetActorLocation());

	if (IsHitFrontal(60.f, this, InstigatorWeapon) && InstigatorWeapon->GetWeaponState() == EWeaponState::EWS_AttackHeavy)
	{
		PlayHitReactMontage(FName("KO"));
	}
	else
	{
		PlayHitReactMontage(HitDirection);
	}

	if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetHitSound())
	{
		UGameplayStatics::PlaySoundAtLocation(this, Combat->GetEquippedWeapon()->GetHitSound(), GetActorLocation());
	}
}

void ABasePlayerCharacter::PlayHitReactMontage(FName Section)
{
	UAnimInstance* CharacterAnimInstance = GetMesh()->GetAnimInstance();
	if (!CharacterAnimInstance) return;

	if (Combat && Combat->GetEquippedWeapon() && Combat->GetEquippedWeapon()->GetHurtMontage())
	{
		CharacterAnimInstance->Montage_Play(Combat->GetEquippedWeapon()->GetHurtMontage(), 1.f);
		CharacterAnimInstance->Montage_JumpToSection(Section);
	}
	else if (UnarmedHurtMontage)
	{
		CharacterAnimInstance->Montage_Play(UnarmedHurtMontage, 1.f);
		CharacterAnimInstance->Montage_JumpToSection(Section);
	}
}

void ABasePlayerCharacter::HandleCosmeticBlock(const AWOGBaseWeapon* InstigatorWeapon)
{
	if (!Combat->GetEquippedWeapon()) return;

	if (Combat->GetEquippedWeapon()->GetBlockSound())
	{
		UGameplayStatics::PlaySoundAtLocation(this, Combat->GetEquippedWeapon()->GetBlockSound(), GetActorLocation());
	}

	if (Combat->GetEquippedWeapon()->GetBlockMontage())
	{
		UAnimInstance* CharacterAnimInstance = GetMesh()->GetAnimInstance();
		if (!CharacterAnimInstance) return;

		if(InstigatorWeapon && InstigatorWeapon->GetWeaponState()==EWeaponState::EWS_AttackLight)
		{
			CharacterAnimInstance->Montage_Play(Combat->GetEquippedWeapon()->GetBlockMontage(), 1.f);
			CharacterAnimInstance->Montage_JumpToSection(FName("Impact"));
		}
		else if (InstigatorWeapon && InstigatorWeapon->GetWeaponState() == EWeaponState::EWS_AttackHeavy)
		{
			Server_SetCharacterState(ECharacterState::ECS_Staggered);
		}
	}
}

void ABasePlayerCharacter::HandleCosmeticWeaponClash()
{

	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Blue, FString("Weapons Clashed!"));
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
	FVector ImpulseDirection = LastHitDirection.GetSafeNormal() * 30000.f;
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