// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WOGBaseBuildable.h"
#include "WOGBuildableLadder.generated.h"

/**
 * 
 */
UCLASS()
class WOG_API AWOGBuildableLadder : public AWOGBaseBuildable
{
	GENERATED_BODY()

public:
	AWOGBuildableLadder();

protected:

	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<USceneComponent> FirstAnchor;

	virtual void SetProperties_Implementation(UStaticMesh* Mesh, UStaticMesh* ExtensionMesh, const float& Health, const float& MaxHeightOffset) override;

	UFUNCTION(BlueprintCallable)
	virtual void ExtendLadder();

};
