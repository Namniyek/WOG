// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "WOGMatchHUD.generated.h"

/**
 * 
 */
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

	UFUNCTION()
	void AddEndgameWidget();

	UFUNCTION()
	void AddAnnouncementWidget(FString AnnouncementStringMain, FString AnnouncementStringSec);

};
