// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "ActorComponents/WOGBuildComponent.h"
#include "WOGBaseBuildItem.generated.h"

class UAGR_ItemComponent;
class UAGR_InventoryManager;

UCLASS()
class WOG_API AWOGBaseBuildItem : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWOGBaseBuildItem();
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void PostInitializeComponents();

protected:
	virtual void BeginPlay() override;

	#pragma region ActorComponents

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAGR_ItemComponent> ItemComponent;

	#pragma endregion

	#pragma region Build Variables
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup")
	FName BuildName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FBuildables BuildData;
	#pragma endregion

	#pragma region Item Functions
	UFUNCTION()
	void OnBuildablePickedUp(UAGR_InventoryManager* Inventory);

	UFUNCTION()
	void OnBuildableEquipped(AActor* User, FName SlotName);

	UFUNCTION()
	void OnBuildableUnequipped(AActor* User, FName SlotName);

	UFUNCTION()
	void OnBuildableUsed(AActor* User, FGameplayTag GameplayTag);

	UFUNCTION()
	void OnBuildableDestroyed();
	#pragma endregion

private:
	virtual void InitBuildableData();

public:	
	UFUNCTION(BlueprintPure)
	FORCEINLINE FBuildables GetBuildData() const { return BuildData; }

};
