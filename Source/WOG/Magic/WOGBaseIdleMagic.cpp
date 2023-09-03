// Fill out your copyright notice in the Description page of Project Settings.


#include "Magic/WOGBaseIdleMagic.h"
#include "Characters/WOGBaseCharacter.h"
#include "Magic/WOGBaseMagic.h"
#include "Libraries/WOGBlueprintLibrary.h"
#include "NiagaraComponent.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"

AWOGBaseIdleMagic::AWOGBaseIdleMagic()
{
	PrimaryActorTick.bCanEverTick = false;
	SetReplicates(true);
	bNetLoadOnClient = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(SceneRoot);

	RightHandEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Right Hand Effect"));
	RightHandEffect->SetupAttachment(GetRootComponent());
	RightHandEffect->bAutoActivate = false;

	LeftHandEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Left Hand Effect"));
	LeftHandEffect->SetupAttachment(GetRootComponent());
	LeftHandEffect->bAutoActivate = false;

	IdleSound = CreateDefaultSubobject<UAudioComponent >(TEXT("Idle Sound"));
	IdleSound->bAutoActivate = false;
	IdleSound->SetupAttachment(GetRootComponent());
}

void AWOGBaseIdleMagic::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority()) return;

	TObjectPtr<AWOGBaseCharacter> Character = Cast<AWOGBaseCharacter>(GetOwner());
	if (Character)
	{
		TObjectPtr<AWOGBaseMagic> EquippedMagic = UWOGBlueprintLibrary::GetEquippedMagic(Character);
		if (EquippedMagic && EquippedMagic->GetMagicData().IdleSound)
		{
			Multicast_HandleInit(
				Character->GetMesh(), 
				EquippedMagic->GetMagicData().RighHandSocket, 
				EquippedMagic->GetMagicData().LeftHandSocket, 
				EquippedMagic->GetMagicData().IdleSound);
		}
	}
}

void AWOGBaseIdleMagic::Destroyed()
{
	IdleSound->FadeOut(0.5f, 0);
}

void AWOGBaseIdleMagic::Multicast_HandleInit_Implementation(USkeletalMeshComponent* Mesh, const FName& RightHand, const FName& LeftHand, USoundCue* NewSound)
{
	RightHandEffect->AttachToComponent(Mesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, RightHand);
	LeftHandEffect->AttachToComponent(Mesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, LeftHand);
	RightHandEffect->Activate();
	LeftHandEffect->Activate();

	IdleSound->SetSound(NewSound);
	IdleSound->Activate(true);
}

