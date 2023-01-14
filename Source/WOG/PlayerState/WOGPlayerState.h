// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "WOGPlayerState.generated.h"

USTRUCT(BlueprintType)
struct FPlayerStats
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	int32 TimesElimmed = 10;

	UPROPERTY(BlueprintReadOnly)
	int32 TotalElimms = 4;

	UPROPERTY(BlueprintReadOnly)
	FString MostElimmedPlayer = FString("your mom");
};

/**
 * 
 */
UCLASS()
class WOG_API AWOGPlayerState : public APlayerState
{
	GENERATED_BODY()

private:
	FPlayerStats PlayerStats;

public:
	UFUNCTION(BlueprintPure)
	FORCEINLINE FPlayerStats GetPlayerStats() { return PlayerStats; }

};
