// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Types/CharacterTypes.h"
#include "WOGPlayerState.generated.h"


USTRUCT(BlueprintType)
struct FPlayerStats
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	int32 TimesElimmed = 10;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	int32 TotalElimms = 4;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	FString MostElimmedPlayer = FString("your mom");

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	FString PlayerWithMostElimms = FString("Player0");
};

/**
 * 
 */
UCLASS()
class WOG_API AWOGPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	AWOGPlayerState();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:

private:

	UPROPERTY(Replicated, VisibleAnywhere) 
	FPlayerStats PlayerStats;

	UPROPERTY(VisibleAnywhere)
	FPlayerCharacterEquipmentSnapshot EquipmentSnapshot;

public:
	UFUNCTION(BlueprintPure)
	FORCEINLINE FPlayerStats GetPlayerStats() { return PlayerStats; }

	void IncreaseTimesElimmed();
	void IncreaseTotalElimms();
	void SetMostElimmedPlayer(FString Player);
	void SetPlayerWithMostElimms(FString Player);

	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetEquipmentSnapshot(const FPlayerCharacterEquipmentSnapshot& NewEquipment) { EquipmentSnapshot = NewEquipment; }
	UFUNCTION(BlueprintPure)
	FORCEINLINE FPlayerCharacterEquipmentSnapshot GetEquipmentSnapshot() const { return EquipmentSnapshot; }

	void RestoreEquipmentFromSnapshot();

};
