// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Data/WOGDataTypes.h"
#include "TargetInterface.generated.h"

class AWOGBaseSquad;

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UTargetInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class WOG_API ITargetInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Target")
	void GetTargetWidgetAttachmentParent(USceneComponent*& OutParentComponent, FName& OutSocketName) const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Target")
	bool IsTargetable(AActor* TargeterActor) const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Combat")
	FVector GetMeleeAttackSlot(const int32& SlotIndex) const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Combat")
	FVector GetRangedAttackSlot(const int32& SlotIndex) const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Combat")
	bool IsCurrentMeleeSquadSlotAvailable() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Combat")
	bool IsCurrentRangedSquadSlotAvailable() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Combat")
	bool IsCurrentEpicSquadSlotAvailable() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Combat")
	void FreeCurrentRangedSquadSlot();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Combat")
	void FreeCurrentMeleeSquadSlot();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Combat")
	void FreeCurrentEpicSquadSlot();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Combat")
	void SetCurrentRangedSquadSlot(AWOGBaseSquad* NewSquad);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Combat")
	void SetCurrentMeleeSquadSlot(AWOGBaseSquad* NewSquad);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Combat")
	void SetCurrentEpicSquadSlot(AWOGBaseSquad* NewSquad);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Combat")
	AWOGBaseSquad* GetCurrentRangedSquadSlot() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Combat")
	AWOGBaseSquad* GetCurrentMeleeSquadSlot() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Combat")
	AWOGBaseSquad* GetCurrentEpicSquadSlot() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Combat")
	bool CanBePossessed() const;
};
