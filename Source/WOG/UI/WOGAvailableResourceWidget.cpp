// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WOGAvailableResourceWidget.h"
#include "PlayerCharacter/BasePlayerCharacter.h"
#include "PlayerController/WOGPlayerController.h"
#include "Components/HorizontalBox.h"
#include "Components/Image.h"
#include "Data/AGRLibrary.h"

void UWOGAvailableResourceWidget::FinishInit()
{
	TObjectPtr<AWOGPlayerController> OwnerPC = Cast<AWOGPlayerController>(GetOwningPlayer());
	if (!OwnerPC) return;

	if (OwnerPC->GetIsAttacker())
	{
		WoodContainer->SetVisibility(ESlateVisibility::Collapsed);
		if (SpecResourceIconTexture)
		{
			SpecResourceIcon->SetBrushFromTexture(SpecResourceIconTexture);
		}
	}

	TObjectPtr<ABasePlayerCharacter> PlayerCharacter = Cast<ABasePlayerCharacter>(GetOwningPlayerPawn());
	if (!PlayerCharacter || !PlayerCharacter->GetCommonInventory()) return;

	CommonInventory = UAGRLibrary::GetInventory((const AActor*) (PlayerCharacter->GetCommonInventory()));
}

bool UWOGAvailableResourceWidget::Initialize()
{
	if (!Super::Initialize()) return false;
	return true;
}
