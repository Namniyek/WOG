// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WOGHuntProgressBar.h"
#include "Components/ProgressBar.h"
#include "CommonTextBlock.h"
#include "Enemies/WOGHuntEnemy.h"
#include "WOG.h"
#include "ActorComponents/WOGAbilitySystemComponent.h"
#include "AbilitySystem/AttributeSets/WOGAttributeSetBase.h"

void UWOGHuntProgressBar::NativeConstruct()
{
	Super::NativeConstruct();

	if (!HuntCharacter) return;

	HuntEnemyName->SetText(HuntCharacter->MinionName);
	HuntCharacter->OnAttributeChangedDelegate.AddDynamic(this, &ThisClass::AttributeChangedCallback);

	if (HuntCharacter->GetAttributeSetBase())
	{
		AttributeChangedCallback(HuntCharacter->GetAttributeSetBase()->GetHealthAttribute(), HuntCharacter->GetAttributeSetBase()->GetHealth(), HuntCharacter->GetAttributeSetBase()->GetMaxHealth());
	}
}

void UWOGHuntProgressBar::AttributeChangedCallback(FGameplayAttribute ChangedAttribute, float NewValue, float MaxValue)
{
	HuntProgressBar->SetPercent(FMath::Clamp((NewValue / MaxValue), 0.f, 1.f));
}
