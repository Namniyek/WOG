// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTags.h"
#include "WOGAnimationCollectionData.generated.h"

/**
 * 
 */
class UAnimMontage;


UCLASS()
class WOG_API UWOGAnimationCollectionData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation Collection")
	TMap<FGameplayTag, UAnimMontage*> AnimationCollection;
};
