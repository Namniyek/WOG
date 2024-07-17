// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WOGPingMarker.generated.h"

class UWidgetComponent;

UCLASS()
class WOG_API AWOGPingMarker : public AActor
{
	GENERATED_BODY()

public:
	AWOGPingMarker();

protected:
	virtual void BeginPlay() override;
};
