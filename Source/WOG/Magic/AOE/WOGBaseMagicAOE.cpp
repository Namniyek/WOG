// Fill out your copyright notice in the Description page of Project Settings.


#include "Magic/AOE/WOGBaseMagicAOE.h"
#include "Net/UnrealNetwork.h"
#include "NiagaraComponent.h"
#include "Components/SphereComponent.h"
#include "Characters/WOGBaseCharacter.h"

// Sets default values
AWOGBaseMagicAOE::AWOGBaseMagicAOE()
{
	PrimaryActorTick.bCanEverTick = false;
	InitialLifeSpan = 3.f;
	SetReplicates(true);
	bNetLoadOnClient = false;
	InitDelay = 1.f;

	OverlapSphere = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapSphere"));
	SetRootComponent(OverlapSphere);
	OverlapSphere->SetSphereRadius(250.f);
	OverlapSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	OverlapSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	OverlapSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECR_Overlap);

	AOEEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("AOE Effect"));
	AOEEffect->SetupAttachment(GetRootComponent());
	AOEEffect->bAutoActivate = true;
}

void AWOGBaseMagicAOE::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWOGBaseMagicAOE, MagicData);
}

void AWOGBaseMagicAOE::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		GetWorldTimerManager().SetTimer(InitDelayTimer, this, &AWOGBaseMagicAOE::Init, InitDelay);
	}

	OverlapSphere->OnComponentBeginOverlap.AddDynamic(this, &AWOGBaseMagicAOE::OnOverlap);
}

void AWOGBaseMagicAOE::Init()
{
	OverlapSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

void AWOGBaseMagicAOE::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == GetOwner()) return;

	TObjectPtr<AWOGBaseCharacter> HitCharacter = Cast<AWOGBaseCharacter>(OtherActor);

	if (HitCharacter)
	{
		HitCharacter->ProcessMagicHit(SweepResult, MagicData);
	}
}
