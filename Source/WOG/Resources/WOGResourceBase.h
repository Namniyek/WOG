// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/ResourcesInterface.h"
#include "WOGResourceBase.generated.h"

class UAGR_ItemComponent;

UCLASS()
class WOG_API AWOGResourceBase : public AActor, public IResourcesInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWOGResourceBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAGR_ItemComponent> ItemComponent;

public:	


};
