// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGMatchHUD.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "WOG/UI/MainAnnouncementWidget.h"
#include "WOG/UI/EndgameWidget.h"

void AWOGMatchHUD::BeginPlay()
{
	UWidgetLayoutLibrary::RemoveAllWidgets(this);

	Super::BeginPlay();

	auto OwnerPC = GetOwningPlayerController();
	if (OwnerPC)
	{
		FInputModeGameOnly InputMode;
		OwnerPC->SetInputMode(InputMode);
		OwnerPC->SetShowMouseCursor(false);
	}
}

void AWOGMatchHUD::AddEndgameWidget()
{
	UWidgetLayoutLibrary::RemoveAllWidgets(this);

	auto OwnerPC = GetOwningPlayerController();
	if (OwnerPC)
	{
		FInputModeUIOnly InputMode;
		OwnerPC->SetInputMode(InputMode);
		OwnerPC->SetShowMouseCursor(true);
	}

	if (!EndgameClass) return;
	Endgame = CreateWidget<UEndgameWidget>(GetOwningPlayerController(), EndgameClass);

	if (!Endgame) return;
	Endgame->AddToViewport();
}

void AWOGMatchHUD::AddAnnouncementWidget(FString AnnouncementStringMain, FString AnnouncementStringSec)
{
	if (!AnnouncementClass) return;
	Announcement = CreateWidget<UMainAnnouncementWidget>(GetOwningPlayerController(), AnnouncementClass);

	if (!Announcement) return;
	Announcement->SetAnnouncementText(AnnouncementStringMain, AnnouncementStringSec);
	Announcement->AddToViewport();

}
