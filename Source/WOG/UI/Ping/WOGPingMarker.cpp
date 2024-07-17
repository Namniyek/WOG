// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGPingMarker.h"

AWOGPingMarker::AWOGPingMarker()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	SetReplicateMovement(false);
}

void AWOGPingMarker::BeginPlay()
{
	Super::BeginPlay();
	
	SetLifeSpan(15.f);
}

