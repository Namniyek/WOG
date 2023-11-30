// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WOGCommonInventory.generated.h"

class UAGR_InventoryManager;

UCLASS()
class WOG_API AWOGCommonInventory : public AActor
{
	GENERATED_BODY()
	
public:	
	AWOGCommonInventory();

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	TObjectPtr<UAGR_InventoryManager> Inventory;

public:

	FORCEINLINE UAGR_InventoryManager* GetInventoryComponent() const { return Inventory; }


};
