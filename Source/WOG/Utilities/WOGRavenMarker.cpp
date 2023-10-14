// Fill out your copyright notice in the Description page of Project Settings.


#include "Utilities/WOGRavenMarker.h"
#include "Components/SphereComponent.h"
#include "GameFramework/RotatingMovementComponent.h"
#include "PlayerController//WOGPlayerController.h"
#include "PlayerCharacter/WOGAttacker.h"
#include "Enemies/WOGRaven.h"
#include "Kismet/GameplayStatics.h"

AWOGRavenMarker::AWOGRavenMarker()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	SetRootComponent(SphereComponent);
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SphereComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	SphereComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	SphereComponent->SetGenerateOverlapEvents(true);
	SphereComponent->SetSphereRadius(350.f);

	RotatingComponent = CreateDefaultSubobject<URotatingMovementComponent>(TEXT("RotatingComponent"));
	RotatingComponent->RotationRate = FRotator(0.f, 50.f, 0.f);
	RotatingComponent->bRotationInLocalSpace = true;

	InitDelay = 3.f;
}

void AWOGRavenMarker::BeginPlay()
{
	Super::BeginPlay();

	FTimerHandle InitHandle;

	if (HasAuthority() && SphereComponent)
	{
		SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnSphereOverlap);
		GetWorld()->GetTimerManager().SetTimer(InitHandle, this, &AWOGRavenMarker::Init, InitDelay, false);
	}

	if (!HasAuthority())
	{
		TObjectPtr<AWOGPlayerController> PC = Cast<AWOGPlayerController>(UGameplayStatics::GetPlayerController(this, 0));
		if (PC)
		{
			SetActorHiddenInGame(!PC->GetIsAttacker());
		}
	}
}

void AWOGRavenMarker::Init()
{
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AWOGRavenMarker::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	bool bIsAttacker = OtherActor->IsA<AWOGAttacker>();
	if (bIsAttacker && GetOwner())
	{
		TObjectPtr<AWOGRaven> RavenOwner = Cast<AWOGRaven>(GetOwner());
		if (!RavenOwner)
		{
			UE_LOG(LogTemp, Error, TEXT("Invalid RavenOwner"));
			return;
		}

		if (RavenOwner->SpawnedMarkers.Contains(this))
		{
			int32 AmountRemoved = RavenOwner->SpawnedMarkers.Remove(this);

			UE_LOG(LogTemp, Warning, TEXT("Items removed: %d, %s"), AmountRemoved, *UEnum::GetValueAsString(GetLocalRole()));
			Destroy();
		}
	}
}


