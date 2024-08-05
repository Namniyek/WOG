// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "WOGMatchTypeData.generated.h"

class UTexture2D;
/**
 * 
 */
UCLASS()
class WOG_API UWOGMatchTypeData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Match Data")
	FText MapDisplayName = FText();

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Match Data")
	FString MapPath = FString();

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Match Data")
	TObjectPtr<UTexture2D> MapThumbnail = nullptr;
};
