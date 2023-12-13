// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Types/CharacterTypes.h"
#include "Data/WOGDataTypes.h"
#include "WOGInspectItemBaseWidget.generated.h"

/**
 * 
 */
UCLASS()
class WOG_API UWOGInspectItemBaseWidget : public UUserWidget
{
	GENERATED_BODY()

private:
	FVendorItemData VendorItemData = FVendorItemData();

public:

	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetVendorItemData(const FVendorItemData& NewData) { VendorItemData = NewData; }
	UFUNCTION(BlueprintPure)
	FORCEINLINE FVendorItemData GetVendorItemData() const { return VendorItemData; }
};
