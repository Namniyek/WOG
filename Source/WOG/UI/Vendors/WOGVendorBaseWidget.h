// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/VendorInterface.h"
#include "WOGVendorBaseWidget.generated.h"

/**
 * 
 */
UCLASS()
class WOG_API UWOGVendorBaseWidget : public UUserWidget, public IVendorInterface
{
	GENERATED_BODY()

	void UpdateAvailableResourceWidget_Implementation();
	
};
