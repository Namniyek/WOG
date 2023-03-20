// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGDefender.h"
#include "WOG/ActorComponents/WOGBuildComponent.h"

AWOGDefender::AWOGDefender()
{
	BuildComponent = CreateDefaultSubobject<UWOGBuildComponent>(TEXT("BuildingComponent"));
	BuildComponent->SetIsReplicated(true);
}

void AWOGDefender::DestroyComponent(UActorComponent* ComponentToDestroy)
{
	if (ComponentToDestroy)
	{
		ComponentToDestroy->DestroyComponent();
	}
}

void AWOGDefender::Server_SpawnBuild_Implementation()
{
	if (!BuildComponent) return;

	BuildComponent->SpawnBuild(
		BuildComponent->BuildTransform,
		BuildComponent->BuildID,
		BuildComponent->CurrentHitActor,
		BuildComponent->CurrentHitComponent
	);
}
