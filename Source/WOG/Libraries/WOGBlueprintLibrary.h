// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "WOGBlueprintLibrary.generated.h"

/**
 * 
 */
class AWOGBaseWeapon;

UCLASS()
class WOG_API UWOGBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static AWOGBaseWeapon* GetEquippedWeapon(const AActor* Owner);

private:
	//Can return nullptr 
	UFUNCTION(BlueprintCallable, BlueprintPure, DisplayName = "Get Equipped Weapon", Category = "WOG Library")
	static AWOGBaseWeapon* K2_GetEquippedWeapon(AActor* Owner)
	{
		return GetEquippedWeapon(Owner);
	};

};
