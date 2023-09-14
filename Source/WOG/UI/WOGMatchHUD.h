// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "WOGMatchHUD.generated.h"

/**
 * 
 */
class UWOG_HUD;

UCLASS()
class WOG_API AWOGMatchHUD : public AHUD
{
	GENERATED_BODY()
	
protected:

	virtual void BeginPlay() override;

public:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> AnnouncementClass;

	UPROPERTY()
	class UMainAnnouncementWidget* Announcement;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> EndgameClass;

	UPROPERTY()
	class UEndgameWidget* Endgame;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> RestartClass;

	UPROPERTY()
	class URestartWidget* Restart;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> WarningClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> StaminaBarClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> HoldProgressBarWidgetClass;

	UFUNCTION()
	void AddEndgameWidget();

	UFUNCTION()
	void AddAnnouncementWidget(FString AnnouncementStringMain, FString AnnouncementStringSec);

	UFUNCTION(BlueprintNativeEvent)
	void ResetHUDAfterRespawn();

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	TObjectPtr<UWOG_HUD> HUDWidget = nullptr;

};
