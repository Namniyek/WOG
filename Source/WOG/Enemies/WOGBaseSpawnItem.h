// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "Types/CharacterTypes.h"
#include "WOGBaseSpawnItem.generated.h"

class UAGR_ItemComponent;
class UAGR_InventoryManager;

UCLASS()
class WOG_API AWOGBaseSpawnItem : public AActor
{
	GENERATED_BODY()
	
public:	
	AWOGBaseSpawnItem();
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void PostInitializeComponents();

protected:
	virtual void BeginPlay() override;

	#pragma region ActorComponents

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAGR_ItemComponent> ItemComponent;

	#pragma endregion

	#pragma region Spawn Variables
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup")
	FName SpawnName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FSpawnables SpawnData;
	#pragma endregion

	#pragma region Item Functions
	UFUNCTION()
	void OnSpawnablePickedUp(UAGR_InventoryManager* Inventory);

	UFUNCTION()
	void OnSpawnableEquipped(AActor* User, FName SlotName);

	UFUNCTION()
	void OnSpawnableUnequipped(AActor* User, FName SlotName);

	UFUNCTION()
	void OnSpawnableUsed(AActor* User, FGameplayTag GameplayTag);

	UFUNCTION()
	void OnSpawnableDestroyed();
	#pragma endregion

private:
	virtual void InitSpawnableData();

public:	
	UFUNCTION(BlueprintPure)
	FORCEINLINE FSpawnables GetSpawnData() const { return SpawnData; }
};
