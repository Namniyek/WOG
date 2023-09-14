// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WOGRoundProgressBar.h"
#include "Components/Image.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInstance.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/AttributeSets/WOGAttributeSetBase.h"


void UWOGRoundProgressBar::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	DynamicProgressBarMaterial = UMaterialInstanceDynamic::Create(ProgressBarMaterial, this);

	ProgressBar->SetBrushFromMaterial(DynamicProgressBarMaterial);

	if (!GetOwningPlayerPawn()) return;
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwningPlayerPawn());
	if (!ASC) return;
	AttributeSet = Cast<UWOGAttributeSetBase>(ASC->GetAttributeSet(UAttributeSet::StaticClass()));
	if (!AttributeSet) return;
}

void UWOGRoundProgressBar::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	SetPercent();
}

void UWOGRoundProgressBar::SetPercent()
{
	if (!AttributeSet) return;
	double Percent = AttributeSet->GetStamina() / AttributeSet->GetMaxStamina();

	DynamicProgressBarMaterial->SetScalarParameterValue(FName("Percent"), Percent);
	SetBarColor(AttributeSet->GetStamina());

	if (Percent == 1.f)
	{
		RemoveFromParent();
	}
}

void UWOGRoundProgressBar::SetBarColor(const double& Percent)
{
	FSlateColor Color = Percent > 20.f ? FSlateColor(StaminaColor) : FSlateColor(LowStaminaColor);
	ProgressBar->SetBrushTintColor(Color);
}


