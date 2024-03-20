// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WOGHuntProgressBar.generated.h"


class AWOGHuntEnemy;
class UProgressBar;
class UCommonTextBlock;
/**
 * 
 */
UCLASS()
class WOG_API UWOGHuntProgressBar : public UUserWidget
{
	GENERATED_BODY()


public:

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AWOGHuntEnemy> HuntCharacter;

protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void AttributeChangedCallback(FGameplayAttribute ChangedAttribute, float NewValue, float MaxValue);

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> HuntProgressBar;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<UCommonTextBlock> HuntEnemyName;
};
