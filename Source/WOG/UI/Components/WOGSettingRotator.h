// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonButtonBase.h"
#include "Interfaces/UIInterface.h"
#include "WOGSettingRotator.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRotatedLeft);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRotatedRight);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRotatorSelectionChanged, const FString&, NewValue);
/**
* A rotator to contain a setting.
*/
UCLASS(meta = (DisableNativeTick))
class WOG_API UWOGSettingRotator : public UCommonButtonBase, public IUIInterface
{
	GENERATED_BODY()
	
public:
	virtual bool Initialize() override;
	virtual FNavigationReply NativeOnNavigation(const FGeometry& MyGeometry, const FNavigationEvent& InNavigationEvent, const FNavigationReply& InDefaultReply) override;

	/** Handle and use controller navigation to rotate text */
	FNavigationDelegate OnNavigation;
	TSharedPtr<SWidget> HandleNavigation(EUINavigation UINavigation);

	/** Manually rotate left. */
	UFUNCTION(BlueprintCallable, Category = "WOG Setting Rotator")
	void RotateLeft();

	/** Manually rotate right. */
	UFUNCTION(BlueprintCallable, Category = "WOG Setting Rotator")
	void RotateRight();

	UPROPERTY(BlueprintAssignable)
	FOnRotatedLeft OnRotatedLeft;

	UPROPERTY(BlueprintAssignable)
	FOnRotatedRight OnRotatedRight;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Category = "WOG Setting Rotator")
	TObjectPtr<UNamedSlot> SettingSlot;

	UPROPERTY(BlueprintAssignable)
	FOnRotatorSelectionChanged OnRotatorSelectionChanged;
	
protected:
	void RotateLeftInternal(bool bFromNavigation) const;
	void RotateRightInternal(bool bFromNavigation) const;

	UFUNCTION()
	void OnSelectionChanged(const FString& NewValue);

	UPROPERTY(BlueprintReadOnly, Category = "WOG Setting Rotator")
	FString CurrentSelection = FString();
};
