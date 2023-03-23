// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BasePlayerCharacter.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "WOGDefender.generated.h"

/**
 * 
 */
UCLASS()
class WOG_API AWOGDefender : public ABasePlayerCharacter
{
	GENERATED_BODY()

public:
	AWOGDefender();
	friend class UWOGBuildComponent;

	UFUNCTION(BlueprintCallable)
	void DestroyComponent(UActorComponent* ComponentToDestroy);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UWOGBuildComponent> BuildComponent;
	
};
