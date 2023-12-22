// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "BuildingInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UBuildingInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class WOG_API IBuildingInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.

public:

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	TArray<UBoxComponent*> ReturnCollisionBoxes();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SetProperties(UStaticMesh* Mesh, UStaticMesh* ExtensionMesh, const float& Health, const float& MaxHeightOffset);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void InteractWithBuild();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void DealDamage(const float& Damage, const AActor* Agressor);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void AddBuildChild(AActor* Actor);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void HandleBuildWalls(const FString& HitBoxName, const AActor* BuiltWall);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void HandleDestroyWalls(const AActor* DestroyedWall);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void AddBuildParent(const AActor* Parent);

};
