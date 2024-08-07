// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemies/WOGPossessableEnemy.h"
#include "WOGMinerGiant.generated.h"

/**
 * 
 */
UCLASS()
class WOG_API AWOGMinerGiant : public AWOGPossessableEnemy
{
	GENERATED_BODY()

protected:

	#pragma region TimeOfDay
	virtual void HandleTODChange() override;
	#pragma endregion

	virtual void ProcessHit(FHitResult Hit, UPrimitiveComponent* WeaponMesh) override;
};
