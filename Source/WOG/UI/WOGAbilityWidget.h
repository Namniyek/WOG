// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "WOGAbilityWidget.generated.h"

class UTextBlock;
class UProgressBar;
class UTexture2D;
class UWidgetAnimation;

/**
 * 
 */
UCLASS()
class WOG_API UWOGAbilityWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<UTextBlock> CooldownText;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<UProgressBar> CooldownProgressBar;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	FTimerHandle CooldownTimer;

	//Called when the AbilityWidget is created, but after setting the Icon, CooldownDurationTime and CooldownTag variables;
	UFUNCTION(BlueprintNativeEvent)
	void InitializeWidget();

	UFUNCTION(BlueprintCallable)
	void StartCooldown();
	void EndCooldown();
	void SetCooldownPercent();

	virtual void NativeTick(const FGeometry& MyGeometry, float DeltaTime) override;

private:

	UPROPERTY()
	TObjectPtr<UTexture2D> Icon = nullptr;

	TObjectPtr<UWidgetAnimation> StartAnim;
	TObjectPtr<UWidgetAnimation> UseAnim;

protected:

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	float CooldownDurationTime = 0.f;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	bool bIsInCooldown = false;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	FGameplayTag CooldownTag = FGameplayTag();


public:
	FORCEINLINE void SetCooldownTime(const float& NewCooldown) { CooldownDurationTime = NewCooldown; }
	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetStartAnim(UWidgetAnimation* NewStartAnim) { StartAnim = NewStartAnim; }
	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetUseAnim(UWidgetAnimation* NewUseAnim) { UseAnim = NewUseAnim; }
	FORCEINLINE void SetIconTexture(UTexture2D* NewIcon) { Icon = NewIcon; }
	FORCEINLINE void SetCooldownTag(const FGameplayTag& NewTag) { CooldownTag = NewTag; }
};
