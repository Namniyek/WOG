// Fill out your copyright notice in the Description page of Project Settings.

#include "Resources/WOGVendor.h"

AWOGVendor::AWOGVendor()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);

}

// Called when the game starts or when spawned
void AWOGVendor::BeginPlay()
{
	Super::BeginPlay();
	
}


