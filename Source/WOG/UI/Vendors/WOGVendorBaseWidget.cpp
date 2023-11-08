// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Vendors/WOGVendorBaseWidget.h"
#include "WOG.h"
#include "Resources/WOGVendor.h"
#include "Types/WOGGameplayTags.h"
#include "CommonTextBlock.h"
#include "Components/ScrollBox.h"
#include "Data/AGRLibrary.h"
#include "UI/Vendors/WOGVendorItem.h"
#include "Weapons/WOGBaseWeapon.h"
#include "Magic/WOGBaseMagic.h"
#include "Consumables/WOGBaseConsumable.h"

void UWOGVendorBaseWidget::NativeConstruct()
{
	Super::NativeConstruct();

	RefreshVendorItems();
	SetVendorName();
}

void UWOGVendorBaseWidget::RefreshVendorItems()
{
	if (!VendorActor)
	{
		UE_LOG(WOGLogUI, Error, TEXT("VendorActor invalid"));
		return;
	} 
	if (!PlayerActor)
	{
		UE_LOG(WOGLogUI, Error, TEXT("PlayerActor invalid"));
		return;
	}

	if (VendorItemsBox)
	{
		VendorItemsBox->ClearChildren();
	}

	TObjectPtr<UAGR_InventoryManager> Inventory = UAGRLibrary::GetInventory(VendorActor);
	if (!Inventory) return;

	TArray<AActor*> OutItems = {};
	int32 Amount = 0;
	Inventory->GetAllItemsOfTagSlotType(VendorActor->ItemTypeFilter, OutItems, Amount);
	if (OutItems.IsEmpty())
	{
		UE_LOG(WOGLogUI, Error, TEXT("Vendor doesn't have any valid items"));
		return;
	}

	for (AActor* Item : OutItems)
	{
		if (!IsValid(VendorActor->VendorItemClass))
		{
			UE_LOG(WOGLogUI, Error, TEXT("VendorItem widget class invalid"));
			return;
		}

		TObjectPtr<UWOGVendorItem> VendorItem = Cast<UWOGVendorItem>(CreateWidget<UUserWidget>(GetOwningPlayer(), VendorActor->VendorItemClass));
		if (!VendorItem)
		{
			UE_LOG(WOGLogUI, Error, TEXT("VendorItem widget invalid"));
			return;
		}

		VendorItem->SetBuyerActor(PlayerActor);
		VendorItem->SetVendorActor(VendorActor);

		//The item is a weapon. Set item data accordingly
		if (VendorActor->ItemTypeFilter == TAG_Inventory_Weapon)
		{

			TObjectPtr<AWOGBaseWeapon> Weapon = Cast<AWOGBaseWeapon>(Item);
			if (Weapon)
			{
				VendorItem->SetItemData(Weapon->GetWeaponData().VendorItemData);
			}
			else
			{
				UE_LOG(WOGLogUI, Error, TEXT("Weapon for ItemData invalid"));
			}
		}

		//The item is a magic. Set item data accordingly
		if (VendorActor->ItemTypeFilter == TAG_Inventory_Magic)
		{
			TObjectPtr<AWOGBaseMagic> Magic = Cast<AWOGBaseMagic>(Item);
			if (Magic)
			{
				VendorItem->SetItemData(Magic->GetMagicData().VendorItemData);
			}
			else
			{
				UE_LOG(WOGLogUI, Error, TEXT("Magic for ItemData invalid"));
			}
		}

		//The item is an item. Set item data accordingly
		if (VendorActor->ItemTypeFilter == TAG_Inventory_Consumable)
		{
			TObjectPtr<AWOGBaseConsumable> Consumable = Cast<AWOGBaseConsumable>(Item);
			if (Consumable)
			{
				VendorItem->SetItemData(Consumable->GetConsumableData().VendorItemData);
			}
			else
			{
				UE_LOG(WOGLogUI, Error, TEXT("Consumable for ItemData invalid"));
			}
		}

		VendorItemsBox->AddChild(VendorItem);
	}
}
