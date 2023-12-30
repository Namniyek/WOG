// Fill out your copyright notice in the Description page of Project Settings.
#include "Resources/WOGBaseResourceGather.h"

AWOGBaseResourceGather::AWOGBaseResourceGather()
{
	PrimaryActorTick.bCanEverTick = false;
}

bool AWOGBaseResourceGather::IsTargetable_Implementation(AActor* TargeterActor) const
{
	return true;
}

