// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/WOGBaseInventoryItem.h"
#include "Engine/DataTable.h"
#include "ActorComponents/WOGBuildComponent.h"
#include "Interfaces/BuildingInterface.h"
#include "WOGBaseBuildItem.generated.h"

class UAGR_InventoryManager;

UCLASS()
class WOG_API AWOGBaseBuildItem : public AWOGBaseInventoryItem, public IBuildingInterface
{
	GENERATED_BODY()
	
public:	
	virtual void PostInitializeComponents();

protected:

	#pragma region Build Variables
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FBuildables BuildData;
	#pragma endregion

	FBuildables ReturnBuildData_Implementation();

private:
	virtual void InitData() override;

public:	
	UFUNCTION(BlueprintPure)
	FORCEINLINE FBuildables GetBuildData() const { return BuildData; }

};
