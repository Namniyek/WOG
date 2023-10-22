// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WOGAbilityContainerWidget.h"
#include "UI/WOGAbilityWidget.h"
#include "Components/Overlay.h"

void UWOGAbilityContainerWidget::AddChildAbility(const int32& AbilityID, UWOGAbilityWidget* WidgetToAdd)
{
	if (!WidgetToAdd) return;

	switch (AbilityID)
	{
	case 1:
		AbilityContainer_1->AddChild(WidgetToAdd);
		break;
	case 2:
		AbilityContainer_2->AddChild(WidgetToAdd);
		break;
	case 3:
		AbilityContainer_3->AddChild(WidgetToAdd);
		break;
	case 4:
		AbilityContainer_4->AddChild(WidgetToAdd);
		break;
	}
}

void UWOGAbilityContainerWidget::RemoveChildAbility(const int32& AbilityID)
{
	switch (AbilityID)
	{
	case 1:
		AbilityContainer_1->ClearChildren();
		break;
	case 2:
		AbilityContainer_2->ClearChildren();
		break;
	case 3:
		AbilityContainer_3->ClearChildren();
		break;
	case 4:
		AbilityContainer_4->ClearChildren();
		break;
	}
}
