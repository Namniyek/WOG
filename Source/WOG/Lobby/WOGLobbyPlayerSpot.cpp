// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGLobbyPlayerSpot.h"
#include "Camera/CameraComponent.h"
#include "WOG/PlayerController/WOGLobbyPlayerController.h"
#include "WOG/GameInstance/WOGGameInstance.h"
#include "Kismet/KismetSystemLibrary.h"
#include "WOG/Lobby/WOGLobbyAvatar.h"

// Sets default values
AWOGLobbyPlayerSpot::AWOGLobbyPlayerSpot()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	PreviewMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("PreviewMesh"));
	PreviewMesh->SetupAttachment(GetRootComponent());

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(GetRootComponent());
	Camera->SetMobility(EComponentMobility::Movable);

	CameraBodyTransform = CreateDefaultSubobject<USceneComponent>(TEXT("BodyTransform"));
	CameraBodyTransform->SetupAttachment(GetRootComponent());
	CameraFaceTransform = CreateDefaultSubobject<USceneComponent>(TEXT("FaceTransform"));
	CameraFaceTransform->SetupAttachment(GetRootComponent());

}

void AWOGLobbyPlayerSpot::BeginPlay()
{
	Super::BeginPlay();
	PreviewMesh->DestroyComponent();
}

void AWOGLobbyPlayerSpot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWOGLobbyPlayerSpot::IsOccupied()
{
	bIsSpotUsed = true;
	OwnerPC = Cast<AWOGLobbyPlayerController>(GetOwner());
	if (OwnerPC)
	{
		if (OwnerPC->GetLobbyAvatar())
		{
			OwnerPC->GetLobbyAvatar()->AvatarTransform = GetActorTransform();
			OwnerPC->GetLobbyAvatar()->SetActorTransform(OwnerPC->GetLobbyAvatar()->AvatarTransform, false, nullptr, ETeleportType::TeleportPhysics);
		}
	}
}

void AWOGLobbyPlayerSpot::IsEmpty()
{
	bIsSpotUsed = false;
	SetOwner(nullptr);
	UWOGGameInstance* GameInstance = Cast<UWOGGameInstance>(GetGameInstance());
	if (GameInstance)
	{
		GameInstance->PlayersMap.Add(UserIndex, FString("empty"));
	}
}

void AWOGLobbyPlayerSpot::ChangeCameraToFace()
{
	FVector FaceLocation = CameraFaceTransform->GetRelativeLocation();
	FRotator FaceRotation = CameraFaceTransform->GetRelativeRotation();
	FLatentActionInfo LatentInfo;
	LatentInfo.CallbackTarget = this;
	LatentInfo.ExecutionFunction = "";
	LatentInfo.Linkage = 0;
	UKismetSystemLibrary::MoveComponentTo(Camera, FaceLocation, FaceRotation, true, true, 0.35f, false, EMoveComponentAction::Move, LatentInfo);
}

void AWOGLobbyPlayerSpot::ChangeCameraToBody()
{
	FVector BodyLocation = CameraBodyTransform->GetRelativeLocation();
	FRotator BodyRotation = CameraBodyTransform->GetRelativeRotation();
	FLatentActionInfo LatentInfo;
	LatentInfo.CallbackTarget = this;
	LatentInfo.ExecutionFunction = "";
	LatentInfo.Linkage = 0;
	UKismetSystemLibrary::MoveComponentTo(Camera, BodyLocation, BodyRotation, true, true, 0.35f, false, EMoveComponentAction::Move, LatentInfo);
}

