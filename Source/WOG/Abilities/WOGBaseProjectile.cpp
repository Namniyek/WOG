// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGBaseProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

AWOGBaseProjectile::AWOGBaseProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile Movement"));

	Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	SetRootComponent(Sphere);

	Sphere->SetSphereRadius(12.f);
	Sphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	Sphere->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	Sphere->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	Sphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);

	ProjectileParticles = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ProjectileParticles"));

}

void AWOGBaseProjectile::BeginPlay()
{
	Super::BeginPlay();
	
}

