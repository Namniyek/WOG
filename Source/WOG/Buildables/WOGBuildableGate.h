// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WOGBaseBuildable.h"
#include "WOGBuildableGate.generated.h"

/**
 * 
 */
UCLASS()
class WOG_API AWOGBuildableGate : public AWOGBaseBuildable
{
	GENERATED_BODY()

protected:

	virtual void SetProperties_Implementation(UStaticMesh* Mesh, UStaticMesh* ExtensionMesh, const float& Health, const float& MaxHeightOffset) override;
	
};
