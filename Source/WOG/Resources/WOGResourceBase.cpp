// Fill out your copyright notice in the Description page of Project Settings.


#include "Resources/WOGResourceBase.h"
#include "Components/AGR_ItemComponent.h"

// Sets default values
AWOGResourceBase::AWOGResourceBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	SetReplicates(true);

	ItemComponent = CreateDefaultSubobject<UAGR_ItemComponent>(TEXT("ItemComponent"));
	ItemComponent->bStackable = true;
}

// Called when the game starts or when spawned
void AWOGResourceBase::BeginPlay()
{
	Super::BeginPlay();
	
}

