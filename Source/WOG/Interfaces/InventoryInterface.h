// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Types/CharacterTypes.h"
#include "GameplayTags.h"
#include "InventoryInterface.generated.h"

class AWOGVendor;

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UInventoryInterface : public UInterface
{
	GENERATED_BODY()
};

/**	
 * 
 */
class WOG_API IInventoryInterface
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
	void BuyItem(const TArray<FCostMap>& CostMap, AWOGVendor* VendorActor, TSubclassOf<AActor> ItemClass, const int32& Amount, bool bIsUpgrade, const int32& NewLevel, const FGameplayTag& ItemTag);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void BackFromWidget(AActor* Actor);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SwitchItem(bool bToCommon, AActor* ItemToSwitch, AActor* PreviousItem, FGameplayTagContainer AuxTagsContainer, TSubclassOf<AActor> ItemClass, const int32& Amount);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void HandleHighlight(bool bIsAllowed);
};
