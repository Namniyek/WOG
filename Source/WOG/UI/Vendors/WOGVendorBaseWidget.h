// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WOGVendorBaseWidget.generated.h"

class UCommonTextBlock;
class UScrollBox;
class AWOGVendor;

UCLASS()
class WOG_API UWOGVendorBaseWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void NativeConstruct() override;

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetVendorName();

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<UCommonTextBlock> VendorName;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UScrollBox> VendorItemsBox;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UScrollBox> VendorWeaponsBox;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UScrollBox> VendorMagicBox;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UScrollBox> VendorMinionsBox;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UScrollBox> VendorBuildablesBox;

	void RefreshVendorItems();

private:
	TObjectPtr<AActor> PlayerActor;
	TObjectPtr<AWOGVendor> VendorActor;

public:
	UFUNCTION(BlueprintPure)
	FORCEINLINE AActor* GetPlayerActor() const { return PlayerActor; }
	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetPlayerActor(AActor* NewPlayerRef) { PlayerActor = NewPlayerRef; }

	UFUNCTION(BlueprintPure)
	FORCEINLINE AWOGVendor* GetVendorActor() const { return VendorActor; }
	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetVendorActor(AWOGVendor* NewVendorRef) { VendorActor = NewVendorRef; }

};
