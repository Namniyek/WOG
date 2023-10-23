// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WOGVendorAvailableResourceWidget.generated.h"

/**
 * 
 */

class UHorizontalBox;
class UTexture2D;
class UImage;
class UAGR_InventoryManager;

UCLASS()
class WOG_API UWOGVendorAvailableResourceWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	virtual bool Initialize() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> SpecResourceIcon;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	TObjectPtr<UAGR_InventoryManager> CommonInventory;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UTexture2D> AttackerSpecResourceTexture;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	bool bIsAttacker = false;
	
};
