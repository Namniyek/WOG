// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemies/WOGMinerGiant.h"
#include "WOG.h"
#include "Interfaces/ResourcesInterface.h"
#include "Kismet/KismetSystemLibrary.h"

void AWOGMinerGiant::ProcessHit(FHitResult Hit, UPrimitiveComponent* WeaponMesh)
{
	if (!Hit.GetActor()) return;

	//Check if we hit resources source
	bool bImplementsInterface = UKismetSystemLibrary::DoesImplementInterface(Hit.GetActor(), UResourcesInterface::StaticClass());
	if (bImplementsInterface)
	{
		IResourcesInterface::Execute_ResourcesHit(Hit.GetActor(), Hit, GetActorLocation(), 2);
		return;
	}
}
