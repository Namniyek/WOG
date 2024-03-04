// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Data/WOGDataTypes.h"
#include "Types/CharacterTypes.h"
#include "SpawnInterface.generated.h"

class AWOGBaseSquad;

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class USpawnInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class WOG_API ISpawnInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void UnpossessMinion();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SetAmountRavenMarkersAvailable(const int32& Amount);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void IncreaseRavenMarkerWidget();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void DecreaseRavenMarkerWidget();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SetSpawnerActive(bool NewActive);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	AWOGBaseSquad* GetEnemyOwnerSquad();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	int32 GetEnemySquadUnitIndex();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SetMovementSpeed(const ECharacterMovementSpeed& NewMovementSpeed);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	float GetAttackRangeValue();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	float GetDefendRangeValue();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	int32 GetComboIndex();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void DefineComboIndex();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	FGameplayTag GetAttackData(int32& TokensNeeded);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	FGameplayTag GetAttackDataAtIndex(const int32& Index, int32& TokensNeeded);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void DefineAttackTagIndex();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	FSpawnables ReturnSpawnData();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void ResetComboIndex();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void IncreaseComboIndex();
};
