// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemies/WOGMinerGiant.h"

#include "Data/TODEnum.h"
#include "Interfaces/ResourcesInterface.h"
#include "Kismet/KismetSystemLibrary.h"

void AWOGMinerGiant::HandleTODChange()
{
	if(!HasAuthority()) return;
	
	switch (CurrentTOD)
	{
	case ETimeOfDay::TOD_Dawn1:
		bCanBePossessed = true;
		break;
	case ETimeOfDay::TOD_Dawn2:
		bCanBePossessed = true;
		break;
	case ETimeOfDay::TOD_Dawn3:
		bCanBePossessed = true;
		break;
	case ETimeOfDay::TOD_Dawn4:
		bCanBePossessed = true;
		break;
	default:
		break;
	}
}

void AWOGMinerGiant::ProcessHit(FHitResult Hit, UPrimitiveComponent* WeaponMesh)
{
	if (!Hit.GetActor()) return;

	//Check if we hit resources source
	bool bImplementsInterface = UKismetSystemLibrary::DoesImplementInterface(Hit.GetActor(), UResourcesInterface::StaticClass());
	if (bImplementsInterface)
	{
		IResourcesInterface::Execute_ResourcesHit(Hit.GetActor(), Hit, GetActorLocation(), 4);
	}
}
