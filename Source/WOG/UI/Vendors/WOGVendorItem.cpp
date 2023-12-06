// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Vendors/WOGVendorItem.h"
#include "WOG.h"
#include "CommonBorder.h"
#include "CommonTextBlock.h"
#include "Components/Image.h"
#include "Data/AGRLibrary.h"
#include "Components/ScrollBox.h"
#include "Components/Spacer.h"
#include "Components/HorizontalBox.h"
#include "Data/WOGGameplayTags.h"
#include "Interfaces/InventoryInterface.h"
#include "Subsystems/WOGUIManagerSubsystem.h"
#include "PlayerCharacter/BasePlayerCharacter.h"
#include "Resources/WOGVendor.h"
#include "Data/WOGDataTypes.h"

void UWOGVendorItem::NativeConstruct()
{
	Super::NativeConstruct();

	ItemName->SetText(FText::FromName(ItemData.DisplayName));

	if (ItemData.ItemIcon)
	{
		ItemIcon->SetBrushFromTexture(ItemData.ItemIcon);
	}

	SetCost(ItemData.CostMap);

	TObjectPtr<UScrollBox> ParentWidget = Cast<UScrollBox>(GetParent());
	if(ParentWidget && Border)
	{
		int32 Index = ParentWidget->GetChildIndex(this);
		FLinearColor AltColor = FLinearColor(0.5f, 0.5f, 0.5f);

		Border->SetBrushColor(Index % 2 > 0 ? FLinearColor::White : AltColor);
	}
}

void UWOGVendorItem::AttemptPurchase()
{
	if (!BuyerActor || !VendorActor) return;
	TObjectPtr<AWOGVendor> Vendor = Cast<AWOGVendor>(VendorActor);
	if (!Vendor) return;

	if (CheckCost(ItemData.CostMap))
	{
		//Can afford
		IInventoryInterface* Interface = Cast<IInventoryInterface>(BuyerActor);
		if (Interface)
		{
			Interface->Execute_BuyItem(BuyerActor, ItemData.CostMap, Vendor, ItemData.ItemClass, ItemData.ItemAmount);
		}
		else
		{
			UE_LOG(WOGLogUI, Error, TEXT("Interface invalid"));
		}
	}
	else
	{
		//Can't afford
		TObjectPtr<UWOGUIManagerSubsystem> UIManager = ULocalPlayer::GetSubsystem<UWOGUIManagerSubsystem>(GetOwningLocalPlayer());
		if (UIManager)
		{
			UIManager->CreateResourceWarningWidget(FString("Resources"));
		}

	}
}

bool UWOGVendorItem::CheckCost(const TArray<FCostMap>& CostMap)
{
	if (!BuyerActor)
	{
		UE_LOG(WOGLogUI, Error, TEXT("BuyerActor invalid"));
		return false;
	}

	TObjectPtr<ABasePlayerCharacter> Player = Cast<ABasePlayerCharacter>(BuyerActor);
	if(!Player) 
	{
		UE_LOG(WOGLogUI, Error, TEXT("Player invalid"));
		return false;
	}

	TObjectPtr<UAGR_InventoryManager> CommonInventory = UAGRLibrary::GetInventory((const AActor*) Player->GetCommonInventory());
	if (!CommonInventory)
	{
		UE_LOG(WOGLogUI, Error, TEXT("CommonInventory invalid"));
		return false;
	}
	
	bool bCanAffordItem = false;

	for (FCostMap Cost : CostMap)
	{
		FText OutNote;
		bCanAffordItem = CommonInventory->HasEnoughItemsWithTagSlotType(Cost.CostTag, Cost.CostAmount, OutNote);
		UE_LOG(WOGLogUI, Display, TEXT("CostTag: %s, CostAmount: %d"), *Cost.CostTag.ToString(), Cost.CostAmount);
		UE_LOG(WOGLogUI, Display, TEXT("%s"), *OutNote.ToString());

		if (!bCanAffordItem) break;
	}

	return bCanAffordItem;
}

void UWOGVendorItem::SetCost(const TArray<FCostMap>& CostMap)
{
	GoldCostContainer->SetVisibility(ESlateVisibility::Collapsed);
	IronCostContainer->SetVisibility(ESlateVisibility::Collapsed);
	SpecCostContainer->SetVisibility(ESlateVisibility::Collapsed);
	SpacerGold->SetVisibility(ESlateVisibility::Collapsed);
	SpacerIron->SetVisibility(ESlateVisibility::Collapsed);
	SpacerSpec->SetVisibility(ESlateVisibility::Collapsed);

	for (FCostMap Cost : CostMap)
	{
		if (Cost.CostTag == TAG_Inventory_Resource_Gold)
		{
			SpacerGold->SetVisibility(ESlateVisibility::Visible);
			GoldCostContainer->SetVisibility(ESlateVisibility::Visible);

			GoldCost->SetText(FText::AsNumber(Cost.CostAmount));
		}
		if (Cost.CostTag == TAG_Inventory_Resource_Iron)
		{
			SpacerIron->SetVisibility(ESlateVisibility::Visible);
			IronCostContainer->SetVisibility(ESlateVisibility::Visible);

			IronCost->SetText(FText::AsNumber(Cost.CostAmount));
		}
		if (Cost.CostTag == TAG_Inventory_Resource_ChaosEssence)
		{
			SpacerSpec->SetVisibility(ESlateVisibility::Visible);
			SpecCostContainer->SetVisibility(ESlateVisibility::Visible);

			SpecResourceCost->SetText(FText::AsNumber(Cost.CostAmount));
			SpecResourceIcon->SetBrushFromTexture(ChaosEssenceIcon.Get());
		}
		if (Cost.CostTag == TAG_Inventory_Resource_Souls)
		{
			SpacerSpec->SetVisibility(ESlateVisibility::Visible);
			SpecCostContainer->SetVisibility(ESlateVisibility::Visible);

			SpecResourceCost->SetText(FText::AsNumber(Cost.CostAmount));
			SpecResourceIcon->SetBrushFromTexture(SoulsIcon.Get());
		}
	}


}
