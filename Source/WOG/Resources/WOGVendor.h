// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTags.h"
#include "WOGVendor.generated.h"

class UWOGVendorItem;

UCLASS()
class WOG_API AWOGVendor : public AActor
{
	GENERATED_BODY()
	
public:	
	AWOGVendor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<USkeletalMeshComponent> Mesh;

public:	

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Setup)
	FGameplayTag ItemTypeFilter;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Setup)
	FGameplayTag VendorTypeFilter;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Setup)
	TMap<TSubclassOf<AActor>, int32> DefaultInventoryMap;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Setup)
	TSubclassOf<UWOGVendorItem> VendorItemClass;
};
