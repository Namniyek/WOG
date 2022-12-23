// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGMatchHUD.h"
#include "Blueprint/WidgetLayoutLibrary.h"

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
