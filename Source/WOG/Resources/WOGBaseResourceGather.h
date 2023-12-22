// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TargetSystemTargetableInterface.h"
#include "WOGBaseResourceGather.generated.h"

UCLASS()
class WOG_API AWOGBaseResourceGather : public AActor, public ITargetSystemTargetableInterface
{
	GENERATED_BODY()
	
public:	
	AWOGBaseResourceGather();

};
