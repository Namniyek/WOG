// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/WOGDataTypes.h"
#include "WOGVendorItem.generated.h"

class UCommonBorder;
class UImage;
class UCommonTextBlock;
class USpacer;
class UHorizontalBox;
class UCommonButtonBase;

/**
 * 
 */
UCLASS()
class WOG_API UWOGVendorItem : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void NativeConstruct() override;

	UFUNCTION(BlueprintCallable)
	void AttemptPurchase();

protected:

	void SetCost(const TArray<FCostMap>& CostMap);
	bool CheckCost(const TArray<FCostMap>& CostMap);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> ChaosEssenceIcon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> SoulsIcon;


private:
	FVendorItemData ItemData;

	TObjectPtr<AActor> BuyerActor;
	TObjectPtr<AActor> VendorActor;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonBorder> Border;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> ItemName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> ItemIcon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USpacer> SpacerGold;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USpacer> SpacerIron;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USpacer> SpacerSpec;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHorizontalBox> GoldCostContainer;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHorizontalBox> IronCostContainer;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHorizontalBox> SpecCostContainer;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> GoldCost;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> IronCost;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> SpecResourceCost;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> SpecResourceIcon;

public:

	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetItemData(const FVendorItemData& NewData) { ItemData = NewData; }
	UFUNCTION(BlueprintPure)
	FORCEINLINE FVendorItemData GetItemData() const { return ItemData; }
	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetBuyerActor(AActor* NewBuyer) { BuyerActor = NewBuyer; }
	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetVendorActor(AActor* NewVendor) { VendorActor = NewVendor; }
};
