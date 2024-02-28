// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/WOGBaseInventoryItem.h"
#include "Engine/DataTable.h"
#include "Types/CharacterTypes.h"
#include "Interfaces/SpawnInterface.h"
#include "WOGBaseSpawnItem.generated.h"

class UAGR_ItemComponent;
class UAGR_InventoryManager;

UCLASS()
class WOG_API AWOGBaseSpawnItem : public AWOGBaseInventoryItem, public ISpawnInterface
{
	GENERATED_BODY()
	
public:	
	virtual void PostInitializeComponents();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	#pragma region Spawn Variables
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated)
	FSpawnables SpawnData;
	#pragma endregion

	FSpawnables ReturnSpawnData_Implementation();

	virtual void InitData() override;

public:	
	UFUNCTION(BlueprintPure)
	FORCEINLINE FSpawnables GetSpawnData() const { return SpawnData; }
};
