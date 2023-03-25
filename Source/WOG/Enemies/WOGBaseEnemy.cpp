// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGBaseEnemy.h"
#include "LockOnTargetComponent.h"
#include "TargetingHelperComponent.h"
#include "WOG/ActorComponents/WOGAttributesComponent.h"

// Sets default values
AWOGBaseEnemy::AWOGBaseEnemy()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	LockOnTarget = CreateDefaultSubobject<ULockOnTargetComponent>(TEXT("LockOnTargetComponent"));
	LockOnTarget->SetIsReplicated(true);
	TargetAttractor = CreateDefaultSubobject<UTargetingHelperComponent>(TEXT("TargetAttractor"));
	TargetAttractor->SetIsReplicated(true);
	Attributes = CreateDefaultSubobject<UWOGAttributesComponent>(TEXT("Attributes"));
	Attributes->SetIsReplicated(true);

}

void AWOGBaseEnemy::BeginPlay()
{
	Super::BeginPlay();
	
}

void AWOGBaseEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

