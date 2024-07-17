// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WOGPingWidget.generated.h"

class UCommonTextBlock;
/**
 * 
 */
UCLASS()
class WOG_API UWOGPingWidget : public UUserWidget
{
	GENERATED_BODY()

private:
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCommonTextBlock> PingDistanceText;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<AActor> PingActor = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	bool bIsPingAttacker = false;

	void InitVisibility();
	
public:
	UFUNCTION(BlueprintCallable)
	FText SetPingDistanceText();

	virtual void NativeOnInitialized() override;

	UFUNCTION()
	void InitPingWidget(bool bIsAttacker, AActor* NewPingActor);
};
