// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Vendors/WOGVendorAvailableResourceWidget.h"
#include "PlayerCharacter/BasePlayerCharacter.h"
#include "PlayerController/WOGPlayerController.h"
#include "Components/HorizontalBox.h"
#include "Components/Image.h"
#include "Data/AGRLibrary.h"

bool UWOGVendorAvailableResourceWidget::Initialize()
{
	if (!Super::Initialize()) return false;

	TObjectPtr<AWOGPlayerController> OwnerPC = Cast<AWOGPlayerController>(GetOwningPlayer());
	if (!OwnerPC) return false;

	bIsAttacker = OwnerPC->GetIsAttacker();

	if (bIsAttacker)
	{
		if (AttackerSpecResourceTexture)
		{
			SpecResourceIcon->SetBrushFromTexture(AttackerSpecResourceTexture);
		}
	}

	TObjectPtr<ABasePlayerCharacter> PlayerCharacter = Cast<ABasePlayerCharacter>(GetOwningPlayerPawn());
	if (!PlayerCharacter || !PlayerCharacter->GetCommonInventory()) return false;

	CommonInventory = UAGRLibrary::GetInventory((const AActor*)(PlayerCharacter->GetCommonInventory()));
	return true;
}
