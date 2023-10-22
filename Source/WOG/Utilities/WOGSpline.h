// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WOGSpline.generated.h"

class USplineComponent;

UCLASS()
class WOG_API AWOGSpline : public AActor
{
	GENERATED_BODY()
	
public:	
	AWOGSpline();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USplineComponent> SplineComponent;

public:
	FORCEINLINE USplineComponent* GetSplineComponent() const { return SplineComponent; }
};
