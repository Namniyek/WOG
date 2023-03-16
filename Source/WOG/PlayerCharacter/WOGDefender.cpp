// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGDefender.h"

void AWOGDefender::DestroyComponent(UActorComponent* ComponentToDestroy)
{
	if (ComponentToDestroy)
	{
		ComponentToDestroy->DestroyComponent();
	}
}
