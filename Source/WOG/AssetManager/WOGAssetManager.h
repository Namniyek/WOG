// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "WOGAssetManager.generated.h"

/**
 * 
 */
UCLASS()
class WOG_API UWOGAssetManager : public UAssetManager
{
	GENERATED_BODY()

public:

	static UWOGAssetManager& Get();

	/** Starts initial load, gets called from InitializeObjectReferences */
	virtual void StartInitialLoading() override;
	
};
