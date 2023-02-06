// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WOGBaseActorComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WOG_API UWOGBaseActorComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UWOGBaseActorComponent();

protected:
	virtual void BeginPlay() override;

public:	
		
};
