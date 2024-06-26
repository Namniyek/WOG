// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WOG/PlayerState/WOGPlayerState.h"
#include "EndgameWidget.generated.h"

/**
 * 
 */
UCLASS()
class WOG_API UEndgameWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	class UTextBlock* ResultText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TimesElimmed;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TotalElimms;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* MostElimmedPlayer;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* PlayerWithMostElimms;
	
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
