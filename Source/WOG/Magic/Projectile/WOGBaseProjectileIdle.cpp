// Fill out your copyright notice in the Description page of Project Settings.


#include "Magic/Projectile/WOGBaseProjectileIdle.h"

// Sets default values
AWOGBaseProjectileIdle::AWOGBaseProjectileIdle()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void AWOGBaseProjectileIdle::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AWOGBaseProjectileIdle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

