// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WOGStashWidget.generated.h"

class AWOGStashBase;
/**
 * 
 */
UCLASS()
class WOG_API UWOGStashWidget : public UUserWidget
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<AActor> PlayerActor;
	UPROPERTY()
	TObjectPtr<AWOGStashBase> StashActor;

public:
	UFUNCTION(BlueprintPure)
	FORCEINLINE AActor* GetPlayerActor() const { return PlayerActor; }
	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetPlayerActor(AActor* NewPlayerRef) { PlayerActor = NewPlayerRef; }

	UFUNCTION(BlueprintPure)
	FORCEINLINE AWOGStashBase* GetStashActor() const { return StashActor; }
	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetStashActor(AWOGStashBase* NewStashRef) { StashActor = NewStashRef; }
};
