// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Magic/WOGBaseMagic.h"
#include "WOGBaseMagicAOE.generated.h"

UCLASS()
class WOG_API AWOGBaseMagicAOE : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWOGBaseMagicAOE();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FMagicDataTable MagicData;

public:
	FORCEINLINE void SetMagicData(const FMagicDataTable& NewData) { MagicData = NewData; }

	UFUNCTION(BlueprintPure)
	FORCEINLINE FMagicDataTable GetMagicData() const { return MagicData; }

};
