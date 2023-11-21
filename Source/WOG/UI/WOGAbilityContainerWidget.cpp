// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WOGAbilityContainerWidget.h"
#include "UI/WOGAbilityWidget.h"
#include "Components/Overlay.h"
#include "WOG.h"

void UWOGAbilityContainerWidget::AddChildAbility(const int32& AbilityID, UWOGAbilityWidget* WidgetToAdd)
{
	if (!WidgetToAdd) return;

	switch (AbilityID)
	{
	case 1:
		AbilityContainer_1->AddChild(WidgetToAdd);
		OverlayInputButton1->SetVisibility(ESlateVisibility::Visible);
		break;
	case 2:
		AbilityContainer_2->AddChild(WidgetToAdd);
		OverlayInputButton2->SetVisibility(ESlateVisibility::Visible);
		break;
	case 3:
		AbilityContainer_3->AddChild(WidgetToAdd);
		OverlayInputButton3->SetVisibility(ESlateVisibility::Visible);
		break;
	case 4:
		AbilityContainer_4->AddChild(WidgetToAdd);
		OverlayInputButton4->SetVisibility(ESlateVisibility::Visible);
		break;
	}
}

void UWOGAbilityContainerWidget::RemoveChildAbility(const int32& AbilityID)
{
	switch (AbilityID)
	{
	case 1:
		AbilityContainer_1->ClearChildren();
		OverlayInputButton1->SetVisibility(ESlateVisibility::Hidden);
		break;
	case 2:
		AbilityContainer_2->ClearChildren();
		OverlayInputButton2->SetVisibility(ESlateVisibility::Hidden);
		break;
	case 3:
		AbilityContainer_3->ClearChildren();
		OverlayInputButton3->SetVisibility(ESlateVisibility::Hidden);
		break;
	case 4:
		AbilityContainer_4->ClearChildren();
		OverlayInputButton4->SetVisibility(ESlateVisibility::Hidden);
		break;
	}
}
