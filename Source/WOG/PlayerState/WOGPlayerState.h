// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "PlayerCharacter/BasePlayerCharacter.h"
#include "Types/CharacterTypes.h"
#include "WOGPlayerState.generated.h"


class ABasePlayerCharacter;

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

private:

	UPROPERTY(Replicated)
	bool bIsAttacker;

	UPROPERTY(Replicated)
	TObjectPtr<ABasePlayerCharacter> PlayerCharacter;
	
	UPROPERTY(Replicated, VisibleAnywhere) 
	FPlayerStats PlayerStats;

	UPROPERTY(VisibleAnywhere)
	FPlayerCharacterEquipmentSnapshot EquipmentSnapshot;

public:
	UFUNCTION(BlueprintPure)
	FORCEINLINE FPlayerStats GetPlayerStats() { return PlayerStats; }

	void IncreaseTimesElimmed();
	void IncreaseTotalElims();
	void SetMostElimmedPlayer(FString Player);
	void SetPlayerWithMostElims(FString Player);

	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetEquipmentSnapshot(const FPlayerCharacterEquipmentSnapshot& NewEquipment) { EquipmentSnapshot = NewEquipment; }
	UFUNCTION(BlueprintPure)
	FORCEINLINE FPlayerCharacterEquipmentSnapshot GetEquipmentSnapshot() const { return EquipmentSnapshot; }
	UFUNCTION(BlueprintPure)
	FORCEINLINE bool GetIsAttacker() const { return bIsAttacker; }
	void SetIsAttacker(const bool bNewAttacker);
	UFUNCTION(BlueprintPure)
	FORCEINLINE ABasePlayerCharacter* GetPlayerCharacter() const { return PlayerCharacter; }
	void SetPlayerCharacter(ABasePlayerCharacter* NewPlayerChar); 
};
