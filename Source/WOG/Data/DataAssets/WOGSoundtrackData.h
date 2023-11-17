// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MetasoundSource.h"
#include "WOGSoundtrackData.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FSoundtrackData
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BaseData")
	FName TrackName = FName("None");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BaseData")
	TSoftObjectPtr<UMetaSoundSource> Track = nullptr;
};


UCLASS()
class WOG_API UWOGSoundtrackData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Day")
	TArray<FSoundtrackData> DayTracks;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Night")
	TArray<FSoundtrackData> NightTracks_Metal;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Night")
	TArray<FSoundtrackData> NightTracks_Fantasy;
};
