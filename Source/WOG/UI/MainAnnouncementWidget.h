// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainAnnouncementWidget.generated.h"

/**
 * 
 */
UCLASS()
class WOG_API UMainAnnouncementWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	virtual bool Initialize() override;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* AnnouncementTextMain;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* AnnouncementTextSec;

	void SetAnnouncementText(FString TextMain, FString TextSec);

	UFUNCTION(BlueprintImplementableEvent)
	void RemoveMainAnnouncement();

private:
	FString StartingText = FString("TEST STRING");

	FTimerHandle DestroyTimer;
	float DestroyTime = 3.f;
	
};
