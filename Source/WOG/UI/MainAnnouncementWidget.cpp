// Fill out your copyright notice in the Description page of Project Settings.


#include "MainAnnouncementWidget.h"
#include "Components/TextBlock.h"

bool UMainAnnouncementWidget::Initialize()
{
	if (!Super::Initialize()) return false;
	AnnouncementTextMain->SetText(FText::FromString(StartingText));
	AnnouncementTextSec->SetText(FText::FromString(StartingText));
	GetWorld()->GetTimerManager().SetTimer(DestroyTimer, this, &ThisClass::RemoveMainAnnouncement, DestroyTime);
	return true;
}

void UMainAnnouncementWidget::SetAnnouncementText(FString TextMain, FString TextSec)
{
	AnnouncementTextMain->SetText(FText::FromString(TextMain));
	AnnouncementTextSec->SetText(FText::FromString(TextSec));
}
