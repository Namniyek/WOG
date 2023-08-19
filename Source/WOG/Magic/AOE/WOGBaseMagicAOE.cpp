// Fill out your copyright notice in the Description page of Project Settings.


#include "Magic/AOE/WOGBaseMagicAOE.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AWOGBaseMagicAOE::AWOGBaseMagicAOE()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

void AWOGBaseMagicAOE::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWOGBaseMagicAOE, MagicData);
}

// Called when the game starts or when spawned
void AWOGBaseMagicAOE::BeginPlay()
{
	Super::BeginPlay();
}
