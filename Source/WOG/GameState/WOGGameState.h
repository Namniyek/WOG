// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "WOGGameState.generated.h"

/**
 * 
 */
UCLASS()
class WOG_API AWOGGameState : public AGameState
{
	GENERATED_BODY()

public:
	int32 CurrentTime = 0;
	float UpdateFrequency = 0.5f;

protected:
	virtual void HandleMatchHasStarted() override;

	UFUNCTION(BlueprintCallable)
	virtual int32 UpdateCurrentTime();

	
};
