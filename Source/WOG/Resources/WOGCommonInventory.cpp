// Fill out your copyright notice in the Description page of Project Settings.


#include "Resources/WOGCommonInventory.h"
#include "Components/AGR_InventoryManager.h"

AWOGCommonInventory::AWOGCommonInventory()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	Inventory = CreateDefaultSubobject<UAGR_InventoryManager>(TEXT("Inventory"));
}


