// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGLobbyAvatar.h"
#include "Net/UnrealNetwork.h"
#include "WOG/PlayerController/WOGLobbyPlayerController.h"
#include "Components/TextRenderComponent.h"
#include "GameFramework/PlayerState.h"
#include "WOG/GameMode/WOGLobbyGameMode.h"
#include "WOG/Lobby/WOGLobbyPlayerSpot.h"
#include "GameFramework/RotatingMovementComponent.h"


AWOGLobbyAvatar::AWOGLobbyAvatar()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	bNetUseOwnerRelevancy = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	PlayerNameText = CreateDefaultSubobject< UTextRenderComponent>(TEXT("PlayerNameText"));
	PlayerNameText->SetupAttachment(GetRootComponent());

	BaseMesh = CreateDefaultSubobject< USkeletalMeshComponent>(TEXT("BaseMesh"));
	BaseMesh->SetupAttachment(GetRootComponent());
	BaseMesh->bHiddenInGame = true;

	Head = CreateDefaultSubobject< USkeletalMeshComponent>(TEXT("Head"));
	Head->SetupAttachment(BaseMesh);

	Torso = CreateDefaultSubobject< USkeletalMeshComponent>(TEXT("Torso"));
	Torso->SetupAttachment(BaseMesh);

	Hips = CreateDefaultSubobject< USkeletalMeshComponent>(TEXT("Hips"));
	Hips->SetupAttachment(BaseMesh);

	ArmUpperLeft = CreateDefaultSubobject< USkeletalMeshComponent>(TEXT("ArmUpperLeft"));
	ArmUpperLeft->SetupAttachment(BaseMesh);

	ArmUpperRight = CreateDefaultSubobject< USkeletalMeshComponent>(TEXT("ArmUpperRight"));
	ArmUpperRight->SetupAttachment(BaseMesh);

	ArmLowerLeft = CreateDefaultSubobject< USkeletalMeshComponent>(TEXT("ArmLowerLeft"));
	ArmLowerLeft->SetupAttachment(BaseMesh);

	ArmLowerRight = CreateDefaultSubobject< USkeletalMeshComponent>(TEXT("ArmLowerRight"));
	ArmLowerRight->SetupAttachment(BaseMesh);

	HandLeft = CreateDefaultSubobject< USkeletalMeshComponent>(TEXT("HandLeft"));
	HandLeft->SetupAttachment(BaseMesh);

	HandRight = CreateDefaultSubobject< USkeletalMeshComponent>(TEXT("HandRight"));
	HandRight->SetupAttachment(BaseMesh);

	LegLeft = CreateDefaultSubobject< USkeletalMeshComponent>(TEXT("LegLeft"));
	LegLeft->SetupAttachment(BaseMesh);

	LegRight = CreateDefaultSubobject< USkeletalMeshComponent>(TEXT("LegRight"));
	LegRight->SetupAttachment(BaseMesh);

	Hair = CreateDefaultSubobject< USkeletalMeshComponent>(TEXT("Hair"));
	Hair->SetupAttachment(BaseMesh);

	Beard = CreateDefaultSubobject< USkeletalMeshComponent>(TEXT("Beard"));
	Beard->SetupAttachment(BaseMesh);

	Ears = CreateDefaultSubobject< USkeletalMeshComponent>(TEXT("Ears"));
	Ears->SetupAttachment(BaseMesh);

	Eyebrows = CreateDefaultSubobject< USkeletalMeshComponent>(TEXT("Eyebrows"));
	Eyebrows->SetupAttachment(BaseMesh);

	Helmet = CreateDefaultSubobject< USkeletalMeshComponent>(TEXT("Helmet"));
	Helmet->SetupAttachment(BaseMesh);

	RotatingComponent = CreateDefaultSubobject<URotatingMovementComponent>(TEXT("RotatingComponent"));
	RotatingComponent->RotationRate = FRotator();
	RotatingComponent->bRotationInLocalSpace = true;

	RuneMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RuneMesh"));
	RuneMesh->SetupAttachment(BaseMesh, FName("HandR_Rune"));
}

void AWOGLobbyAvatar::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	DynamicAvatarMaterial = UMaterialInstanceDynamic::Create(Material, this);
	RuneMesh->SetVisibility(false);
}

void AWOGLobbyAvatar::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWOGLobbyAvatar, MeshProperties);
	DOREPLIFETIME(AWOGLobbyAvatar, AvatarTransform);
}

void AWOGLobbyAvatar::BeginPlay()
{
	Super::BeginPlay();

	FTimerHandle StartDelay;
	GetWorldTimerManager().SetTimer(StartDelay, this, &ThisClass::OnStartDelayFinished, 0.5f);
}

void AWOGLobbyAvatar::OnStartDelayFinished()
{
	OwnerPC = Cast<AWOGLobbyPlayerController>(GetOwner());
	if (!OwnerPC) return;

	PlayerName = FText::FromString(OwnerPC->PlayerState->GetPlayerName());

	FPlayerData DefaultAvatar;
	DefaultAvatar.bIsMale = true;
	DefaultAvatar.bIsAttacker = OwnerPC->bIsAttacker;
	DefaultAvatar.PlayerName = OwnerPC->PlayerState->GetPlayerName();
	DefaultAvatar.CharacterIndex = "0";
	DefaultAvatar.PrimaryColor = "0";
	DefaultAvatar.SecondaryColor = "0";
	DefaultAvatar.SkinColor = "0";
	DefaultAvatar.BodyPaintColor = "0";
	DefaultAvatar.HairColor = "0";
	DefaultAvatar.Rune = "0";

	Server_SetMeshProperties(DefaultAvatar);
}

void AWOGLobbyAvatar::Server_SetMeshProperties_Implementation(FPlayerData NewMeshProperties)
{
	MeshProperties = NewMeshProperties;
	UpdateCharacterMesh(MeshProperties);
}

void AWOGLobbyAvatar::OnRep_MeshProperties()
{
	UpdateCharacterMesh(MeshProperties);
}

void AWOGLobbyAvatar::SetPlayerName(FText NewPlayerName)
{
	PlayerNameText->SetText(NewPlayerName);
}

void AWOGLobbyAvatar::SetTransform(FTransform NewTransform)
{
	SetActorTransform(NewTransform, false, nullptr, ETeleportType::ResetPhysics);
}

void AWOGLobbyAvatar::OnRep_AvatarTransform()
{
	SetTransform(AvatarTransform);
}

void AWOGLobbyAvatar::Server_ChangeTeams_Implementation()
{
	bool bWasSucessful = ChangeTeams(OwnerPC);
}

bool AWOGLobbyAvatar::ChangeTeams(AWOGLobbyPlayerController* OwnerPlayerController)
{
	if (!OwnerPlayerController) return false;

	AWOGLobbyGameMode* GameMode = Cast<AWOGLobbyGameMode>(GetWorld()->GetAuthGameMode());
	if (!GameMode) return false;
	bool bGotSpot;

	switch (OwnerPlayerController->GetLobbyPlayerSpot()->TeamIndex)
	{
		case 0:
			AWOGLobbyPlayerSpot * NewAttackerSpot;
			bGotSpot = GameMode->GetAttackerPlayerSpot(NewAttackerSpot);
			if (!bGotSpot) return false;

			OwnerPlayerController->GetLobbyPlayerSpot()->IsEmpty();
			OwnerPlayerController->SetLobbyPlayerSpot(nullptr);
			if (NewAttackerSpot)
			{
				OwnerPlayerController->SetLobbyPlayerSpot(NewAttackerSpot);
				OwnerPlayerController->GetLobbyPlayerSpot()->SetOwner(OwnerPlayerController);
				OwnerPlayerController->GetLobbyPlayerSpot()->IsOccupied();
				OwnerPlayerController->Server_SetIsAttacker(true);
				return true;
			}
			break;

		case 1:
			AWOGLobbyPlayerSpot * NewDefenderSpot;
			bGotSpot = GameMode->GetDefenderPlayerSpot(NewDefenderSpot);
			if (!bGotSpot) return false;

			OwnerPlayerController->GetLobbyPlayerSpot()->IsEmpty();
			OwnerPlayerController->SetLobbyPlayerSpot(nullptr);
			if (NewDefenderSpot)
			{
				OwnerPlayerController->SetLobbyPlayerSpot(NewDefenderSpot);
				OwnerPlayerController->GetLobbyPlayerSpot()->SetOwner(OwnerPlayerController);
				OwnerPlayerController->GetLobbyPlayerSpot()->IsOccupied();
				OwnerPlayerController->Server_SetIsAttacker(false);
				return true;
			}
			break;
	}
	return false;
}

void AWOGLobbyAvatar::RotateLeftStart()
{
	if (RotatingComponent)
	{
		RotatingComponent->RotationRate = FRotator(0.f, 60.f, 0.f);
	}
}

void AWOGLobbyAvatar::RotateLeftStop()
{
	if (RotatingComponent)
	{
		RotatingComponent->RotationRate = FRotator(0.f, 0.f, 0.f);
	}
}

void AWOGLobbyAvatar::RotateRightStart()
{
	if (RotatingComponent)
	{
		RotatingComponent->RotationRate = FRotator(0.f, -60.f, 0.f);
	}
}

void AWOGLobbyAvatar::RotateRightStop()
{
	if (RotatingComponent)
	{
		RotatingComponent->RotationRate = FRotator(0.f, 0.f, 0.f);
	}
}

void AWOGLobbyAvatar::RaiseArm()
{
	if (!RuneMesh) return;
	RuneMesh->SetVisibility(true);
	UAnimInstance* AnimInstance = BaseMesh->GetAnimInstance();
	if (!AnimInstance || !LobbyMontage)
	{
		return;
	}

	AnimInstance->Montage_Play(LobbyMontage);
	AnimInstance->Montage_JumpToSection(FName("ArmRaise"));

}

void AWOGLobbyAvatar::LowerArm()
{
	if (!RuneMesh) return;
	RuneMesh->SetVisibility(false);
	UAnimInstance* AnimInstance = BaseMesh->GetAnimInstance();
	if (!AnimInstance || !LobbyMontage)
	{
		return;
	}

	AnimInstance->Montage_Play(LobbyMontage);
	AnimInstance->Montage_JumpToSection(FName("ArmLower"));
}

