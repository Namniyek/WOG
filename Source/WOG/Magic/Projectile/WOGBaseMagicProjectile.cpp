// Fill out your copyright notice in the Description page of Project Settings.


#include "Magic/Projectile/WOGBaseMagicProjectile.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AWOGBaseMagicProjectile::AWOGBaseMagicProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

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
	
}

