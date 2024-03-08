// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "WOGMatchHUD.generated.h"

/**
 * 
 */
class UWOG_HUD;
class UWOGRavenMarkerWidget;
class UWOGScreenDamage;
class UWOGWarningWidget;
class UMainAnnouncementWidget;
class UEndgameWidget;
class URestartWidget;
class UWOGAvailableResourceWidget;
class UWOGVendorBaseWidget;
class UWOGStashWidget;

UCLASS()
class WOG_API AWOGMatchHUD : public AHUD
{
	GENERATED_BODY()
	
protected:

	virtual void BeginPlay() override;

public:

	#pragma region Widget Classes
	UPROPERTY(EditDefaultsOnly, Category = "Setup | WidgetClasses")
	TSubclassOf<UUserWidget> AnnouncementClass;

	UPROPERTY(EditDefaultsOnly, Category = "Setup | WidgetClasses")
	TSubclassOf<UUserWidget> EndgameClass;

	UPROPERTY(EditDefaultsOnly, Category = "Setup | WidgetClasses")
	TSubclassOf<UUserWidget> RestartClass;

	UPROPERTY(EditDefaultsOnly, Category = "Setup | WidgetClasses")
	TSubclassOf<UWOGWarningWidget> AttributeWarningClass;

	UPROPERTY(EditDefaultsOnly, Category = "Setup | WidgetClasses")
	TSubclassOf<UWOGWarningWidget> GenericWarningClass;

	UPROPERTY(EditDefaultsOnly, Category = "Setup | WidgetClasses")
	TSubclassOf<UUserWidget> StaminaBarClass;

	UPROPERTY(EditDefaultsOnly, Category = "Setup | WidgetClasses")
	TSubclassOf<UUserWidget> HoldProgressBarWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Setup | WidgetClasses")
	TSubclassOf<UWOGRavenMarkerWidget> RavenMarkerWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Setup | WidgetClasses")
	TSubclassOf<UWOGScreenDamage> ScreenDamageWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Setup | WidgetClasses")
	TSubclassOf<UUserWidget> BarsWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Setup | WidgetClasses")
	TSubclassOf<UUserWidget> MinimapWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Setup | WidgetClasses")
	TSubclassOf<UUserWidget> TODWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Setup | WidgetClasses")
	TSubclassOf<UUserWidget> AbilitiesContainerWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Setup | WidgetClasses")
	TSubclassOf<UWOGAvailableResourceWidget> AvailableResourceWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Setup | WidgetClasses")
	TSubclassOf<UWOGVendorBaseWidget> VendorWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Setup | WidgetClasses")
	TSubclassOf<UWOGStashWidget> StashWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Setup | WidgetClasses")
	TSubclassOf<UUserWidget> SquadOrderWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Setup | WidgetClasses")
	TSubclassOf<UUserWidget> CrosshairWidgetClass;

	#pragma endregion

	#pragma region Widget References

	UPROPERTY()
	TObjectPtr<UMainAnnouncementWidget> Announcement;

	UPROPERTY()
	TObjectPtr<UEndgameWidget> Endgame;

	UPROPERTY()
	TObjectPtr<URestartWidget> Restart;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	TObjectPtr<UWOG_HUD> HUDWidget = nullptr;

	#pragma endregion

	UFUNCTION()
	void AddEndgameWidget();

	UFUNCTION()
	void AddAnnouncementWidget(FString AnnouncementStringMain, FString AnnouncementStringSec);

	UFUNCTION(BlueprintNativeEvent)
	void ResetHUDAfterRespawn();



};
