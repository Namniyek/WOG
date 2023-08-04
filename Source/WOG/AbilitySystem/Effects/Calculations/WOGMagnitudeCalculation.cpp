// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Effects/Calculations/WOGMagnitudeCalculation.h"
#include "AbilitySystemComponent.h"
#include "Libraries/WOGBlueprintLibrary.h"
#include "Magic/WOGBaseMagic.h"

float UWOGMagnitudeCalculation::CalculateMagicCost(const FGameplayEffectSpec& Spec) const
{
	TObjectPtr<AActor> Owner = Spec.GetEffectContext().GetInstigatorAbilitySystemComponent()->GetOwner();
	if (Owner)
	{
		TObjectPtr<AWOGBaseMagic> Magic = UWOGBlueprintLibrary::GetEquippedMagic(Owner);
		if (Magic)
		{
			return -Magic->GetMagicData().Cost;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("No magic for cost calculation"));
			return 0.0f;
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("No owner for cost calculation"));
		return 0.0f;
	}
}

float UWOGMagnitudeCalculation::CalculateMagicCooldown(const FGameplayEffectSpec& Spec) const
{
	TObjectPtr<AActor> Owner = Spec.GetEffectContext().GetInstigatorAbilitySystemComponent()->GetOwner();
	if (Owner)
	{
		TObjectPtr<AWOGBaseMagic> Magic = UWOGBlueprintLibrary::GetEquippedMagic(Owner);
		if (Magic)
		{
			return -Magic->GetMagicData().Cooldown;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("No magic for cooldown calculation"));
			return 0.0f;
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("No owner for cooldown calculation"));
		return 0.0f;
	}
}
