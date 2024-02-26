// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "WOGSpawnCosmetics.generated.h"

class USoundCue;
class UNiagaraSystem;

/**
 * 
 */
UCLASS()
class WOG_API UWOGSpawnCosmetics : public UDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
	TObjectPtr<USoundCue> SpawnSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
	TObjectPtr<UNiagaraSystem> SpawnParticleSystem = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Destroy")
	TObjectPtr<USoundCue> DestroySound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Destroy")
	TObjectPtr<UNiagaraSystem> DestroyParticleSystem = nullptr;
	
};
