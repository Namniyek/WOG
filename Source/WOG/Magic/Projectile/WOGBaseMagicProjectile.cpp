// Fill out your copyright notice in the Description page of Project Settings.


#include "Magic/Projectile/WOGBaseMagicProjectile.h"
#include "Net/UnrealNetwork.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/SphereComponent.h"
#include "Characters/WOGBaseCharacter.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/ProjectileMovementComponent.h"


AWOGBaseMagicProjectile::AWOGBaseMagicProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	SetReplicates(true);
	SetReplicateMovement(true);
	bNetLoadOnClient = false;

	OverlapSphere = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapSphere"));
	SetRootComponent(OverlapSphere);
	OverlapSphere->SetSphereRadius(24.f);

	OverlapSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	OverlapSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	OverlapSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECR_Block);
	OverlapSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECR_Block);
	OverlapSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECR_Block);
	OverlapSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Destructible, ECR_Block);
	OverlapSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_PhysicsBody, ECR_Block);


	ProjectileEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("AOE Effect"));
	ProjectileEffect->SetupAttachment(GetRootComponent());
	ProjectileEffect->bAutoActivate = true;

	ProjectileSound = CreateDefaultSubobject<UAudioComponent >(TEXT("Projectile Sound"));
	ProjectileSound->bAutoActivate = true;
	ProjectileSound->SetupAttachment(GetRootComponent());

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile Movement"));
	ProjectileMovement->InitialSpeed = 850.f;
	ProjectileMovement->MaxSpeed = 850.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->ProjectileGravityScale = 0.f;
	ProjectileMovement->bShouldBounce = true;
}

void AWOGBaseMagicProjectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWOGBaseMagicProjectile, MagicData);
}

// Called when the game starts or when spawned
void AWOGBaseMagicProjectile::BeginPlay()
{
	Super::BeginPlay();

	OverlapSphere->OnComponentHit.AddDynamic(this, &AWOGBaseMagicProjectile::OnHit);
}

void AWOGBaseMagicProjectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor == GetOwner()) return;

	TObjectPtr<AWOGBaseCharacter> HitCharacter = Cast<AWOGBaseCharacter>(OtherActor);

	if (HitCharacter)
	{
		HitCharacter->ProcessMagicHit(Hit, MagicData);
	}

	ProjectileEffect->Deactivate();
	ProjectileSound->Stop();
	ProjectileMovement->StopMovementImmediately();

	UGameplayStatics::PlaySoundAtLocation(this, MagicData.HitSound, Hit.ImpactPoint);
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, MagicData.HitVFX, Hit.ImpactPoint);

	SetLifeSpan(0.2f);
}

