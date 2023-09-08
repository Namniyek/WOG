// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WOGAbilityWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Animation/WidgetAnimation.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Internationalization/Text.h"

void UWOGAbilityWidget::InitializeWidget_Implementation()
{
	if (Icon)
	{
		FSlateBrush BackgroundImage;
		BackgroundImage.SetResourceObject(Icon);
		BackgroundImage.TintColor = FSlateColor(FLinearColor(0.1f, 0.1f, 0.1f));
		BackgroundImage.SetImageSize(FVector2D(256, 256));

		FSlateBrush FillImage;
		FillImage.SetResourceObject(Icon);
		FillImage.SetImageSize(FVector2D(256, 256));

		FProgressBarStyle Style;
		Style.SetBackgroundImage(BackgroundImage);
		Style.SetFillImage(FillImage);

		CooldownProgressBar->SetWidgetStyle(Style);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid icon pointer"));
	}

	if (StartAnim)
	{
		PlayAnimationForward(StartAnim);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid StartAnim pointer"));
	}

	CooldownText->SetVisibility(ESlateVisibility::Hidden);
}

void UWOGAbilityWidget::StartCooldown()
{
	if (bIsInCooldown) return;

	bIsInCooldown = true;

	if (UseAnim)
	{
		PlayAnimationForward(UseAnim);
	}
	
	CooldownProgressBar->SetPercent(0.f);

	CooldownText->SetVisibility(ESlateVisibility::Visible);

	GetWorld()->GetTimerManager().SetTimer(CooldownTimer, this, &ThisClass::EndCooldown, CooldownDurationTime);
}

void UWOGAbilityWidget::EndCooldown()
{
	bIsInCooldown = false;

	if (CooldownTimer.IsValid())
	{
		UWorld* World = GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull);
		if (World)
		{
			World->GetTimerManager().ClearTimer(CooldownTimer);
		}
	}

	CooldownProgressBar->SetPercent(1.f);

	if (StartAnim)
	{
		PlayAnimationForward(StartAnim);
	}

	CooldownText->SetVisibility(ESlateVisibility::Hidden);
}

void UWOGAbilityWidget::SetCooldownPercent()
{
	if (!bIsInCooldown || !CooldownTimer.IsValid()) return;

	float TimeElapsed = UKismetSystemLibrary::K2_GetTimerElapsedTimeHandle(this, CooldownTimer);
	float TimeRemaining = UKismetSystemLibrary::K2_GetTimerRemainingTimeHandle(this, CooldownTimer);
	float TotalTime = TimeElapsed + TimeRemaining;

	double Percent = UKismetMathLibrary::NormalizeToRange(TimeElapsed, 0.f, TotalTime);

	CooldownProgressBar->SetPercent(Percent);

	FNumberFormattingOptions Options;
	Options.MaximumFractionalDigits = 1;
	FText Text = FText::AsNumber(TimeRemaining, &Options);
	CooldownText->SetText(Text);
}

void UWOGAbilityWidget::NativeTick(const FGeometry& MyGeometry, float DeltaTime)
{
	Super::NativeTick(MyGeometry, DeltaTime);

	SetCooldownPercent();
}
