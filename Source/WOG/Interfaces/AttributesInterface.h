// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "AttributesInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UAttributesInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class WOG_API IAttributesInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SetMaxHealth(const float& Value);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void BroadcastHit(AActor* AggressorActor, const FHitResult& Hit, const float& DamageToApply, AActor* InstigatorWeapon);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void BroadcastMagicHit(AActor* AggressorActor, const FHitResult& Hit, const struct FMagicDataTable& AggressorMagicData);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool ReserveAttackTokens(const int32& AmountToReserve);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void RestoreAttackTokens(const int32& AmountToRestore);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	int32 GetAvailableAttackTokens();
};
