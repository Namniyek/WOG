// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Types/CharacterTypes.h"
#include "VendorInterface.generated.h"

class AWOGVendor;

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UVendorInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class WOG_API IVendorInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void UpdateAvailableResourceWidget();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void UpdateVendorInventoryWidget();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void TransactionComplete();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void BuyItem(const TArray<FCostMap>& CostMap, AWOGVendor* VendorActor, TSubclassOf<AActor> ItemClass, const int32& Amount);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void BackFromVendorWidget(AActor* Actor);
};
