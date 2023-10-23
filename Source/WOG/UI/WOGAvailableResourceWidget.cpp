// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WOGAvailableResourceWidget.h"
#include "PlayerCharacter/BasePlayerCharacter.h"
#include "PlayerController/WOGPlayerController.h"
#include "Components/HorizontalBox.h"
#include "Components/Image.h"
#include "Data/AGRLibrary.h"

bool UWOGAvailableResourceWidget::Initialize()
{
	if (!Super::Initialize()) return false;
	
	TObjectPtr<AWOGPlayerController> OwnerPC = Cast<AWOGPlayerController>(GetOwningPlayer());
	if (!OwnerPC) return false;

	if (OwnerPC->GetIsAttacker())
	{
		WoodContainer->SetVisibility(ESlateVisibility::Collapsed);
		if (SpecResourceIconTexture)
		{
			SpecResourceIcon->SetBrushFromTexture(SpecResourceIconTexture);
		}
	}

	TObjectPtr<ABasePlayerCharacter> PlayerCharacter = Cast<ABasePlayerCharacter>(GetOwningPlayerPawn());
	if (!PlayerCharacter || !PlayerCharacter->GetCommonInventory()) return false;

	CommonInventory = UAGRLibrary::GetInventory((const AActor*) (PlayerCharacter->GetCommonInventory()));
	return true;
}
