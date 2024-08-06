// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "WOG/PlayerState/WOGPlayerState.h"
#include "EndgameWidget.generated.h"

/**
 * 
 */
UCLASS()
class WOG_API UEndgameWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	class UCommonTextBlock* ResultText;

	UPROPERTY(meta = (BindWidget))
	UCommonTextBlock* TimesElimmed;

	UPROPERTY(meta = (BindWidget))
	UCommonTextBlock* TotalElimms;

	UPROPERTY(meta = (BindWidget))
	UCommonTextBlock* MostElimmedPlayer;

	UPROPERTY(meta = (BindWidget))
	UCommonTextBlock* PlayerWithMostElimms;
	
	UPROPERTY(BlueprintReadOnly)
	FPlayerStats PlayerStats;

protected:
	virtual void NativeOnInitialized() override;

private:
	UPROPERTY()
	AWOGPlayerState* PlayerState;

	UPROPERTY()
	class AWOGGameState* GameState;

	FPlayerStats GetPlayerStats();
	UFUNCTION(BlueprintCallable)
	bool IsWinnerAttacker();

	void SetResults();
	
};
