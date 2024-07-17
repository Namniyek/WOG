// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGPingWidget.h"

#include "CommonTextBlock.h"
#include "PlayerController/WOGPlayerController.h"

FText UWOGPingWidget::SetPingDistanceText()
{
	if(!PingActor) return FText();
	if(!GetOwningPlayer() || !GetOwningPlayer()->GetPawn()) return FText();

	float Distance = (PingActor->GetActorLocation() - GetOwningPlayer()->GetPawn()->GetActorLocation()).Length()/100.f;
	FNumberFormattingOptions Options;
	Options.SetMinimumFractionalDigits(0);
	Options.SetMaximumFractionalDigits(0);
	Options.RoundingMode = ToZero;
	FText DistanceText = FText::AsNumber(Distance, &Options);
	return DistanceText;
}

void UWOGPingWidget::InitVisibility()
{
	const AWOGPlayerController* OwnerPC = Cast<AWOGPlayerController>(GetOwningPlayer());
	if(!OwnerPC) return;
	
	ESlateVisibility NewVisibility = OwnerPC->GetIsAttacker() == bIsPingAttacker ? ESlateVisibility::Visible : ESlateVisibility::Hidden;
	SetVisibility(NewVisibility);
}

void UWOGPingWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	InitVisibility();
}

void UWOGPingWidget::InitPingWidget(bool bIsAttacker, AActor* NewPingActor)
{
	bIsPingAttacker = bIsAttacker;
	PingActor = NewPingActor;

	InitVisibility();
}
