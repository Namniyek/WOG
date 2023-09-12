// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WOGWarningWidget.h"
#include "Components/TextBlock.h"

void UWOGWarningWidget::SetWarningText(const FString& Attribute)
{
	FString WarningString = FString("Not enough ") + Attribute;
	WarningText->SetText(FText::FromString(WarningString));
}
