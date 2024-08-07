// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "UIInterface.generated.h"

class UCommonTextStyle;
// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UUIInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class WOG_API IUIInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SelectNext();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SelectPrevious();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SetTextStyle(TSubclassOf<UCommonTextStyle> NewStyle);
};
