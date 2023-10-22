// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Effects/Calculations/WOGMagnitudeCalculation.h"
#include "AbilitySystemComponent.h"
#include "Libraries/WOGBlueprintLibrary.h"
#include "Magic/WOGBaseMagic.h"
#include "Weapons/WOGBaseWeapon.h"

float UWOGMagnitudeCalculation::CalculateManaCost(const FGameplayEffectSpec& Spec) const
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

float UWOGMagnitudeCalculation::CalculateWeaponLightAttackCost(const FGameplayEffectSpec& Spec) const
{
	TObjectPtr<AActor> Owner = Spec.GetEffectContext().GetInstigatorAbilitySystemComponent()->GetOwner();
	if (Owner)
	{
		TObjectPtr<AWOGBaseWeapon> Weapon = UWOGBlueprintLibrary::GetEquippedWeapon(Owner);
		if (Weapon)
		{
			return -Weapon->GetWeaponData().Cost;
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

float UWOGMagnitudeCalculation::CalculateWeaponHeavyAttackCost(const FGameplayEffectSpec& Spec) const
{
	TObjectPtr<AActor> Owner = Spec.GetEffectContext().GetInstigatorAbilitySystemComponent()->GetOwner();
	if (Owner)
	{
		TObjectPtr<AWOGBaseWeapon> Weapon = UWOGBlueprintLibrary::GetEquippedWeapon(Owner);
		if (Weapon)
		{
			return -Weapon->GetWeaponData().Cost * Weapon->GetWeaponData().HeavyDamageMultiplier;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("No weapon for cost calculation"));
			return 0.0f;
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("No owner for cost calculation"));
		return 0.0f;
	}
}

float UWOGMagnitudeCalculation::CalculateWeaponBlockCost(const FGameplayEffectSpec& Spec) const
{
	TObjectPtr<AActor> Owner = Spec.GetEffectContext().GetInstigatorAbilitySystemComponent()->GetOwner();
	if (Owner)
	{
		TObjectPtr<AWOGBaseWeapon> Weapon = UWOGBlueprintLibrary::GetEquippedWeapon(Owner);
		if (Weapon)
		{
			return -Weapon->GetWeaponData().Cost / 2;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("No weapon for cost calculation"));
			return 0.0f;
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("No owner for cost calculation"));
		return 0.0f;
	}
}

float UWOGMagnitudeCalculation::CalculateMagicSecondaryEffectDuration(const FGameplayEffectSpec& Spec) const
{
	UE_LOG(LogTemp, Warning, TEXT("Secondary Magic effect duration: %f"), Spec.GetLevel());
	return Spec.GetLevel();
}

float UWOGMagnitudeCalculation::CalculateMagicValue(const FGameplayEffectSpec& Spec) const
{
	TObjectPtr<AActor> Owner = Spec.GetEffectContext().GetInstigatorAbilitySystemComponent()->GetOwner();
	if (Owner)
	{
		TObjectPtr<AWOGBaseMagic> Magic = UWOGBlueprintLibrary::GetEquippedMagic(Owner);
		if (Magic)
		{
			return Magic->GetMagicData().Value * Magic->GetMagicData().ValueMultiplier;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("No magic for value calculation"));
			return 0.0f;
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("No owner for cooldown calculation"));
		return 0.0f;
	}
}

float UWOGMagnitudeCalculation::CalculateMagicDuration(const FGameplayEffectSpec& Spec) const
{
	TObjectPtr<AActor> Owner = Spec.GetEffectContext().GetInstigatorAbilitySystemComponent()->GetOwner();
	if (Owner)
	{
		TObjectPtr<AWOGBaseMagic> Magic = UWOGBlueprintLibrary::GetEquippedMagic(Owner);
		if (Magic)
		{
			return Magic->GetMagicData().Duration;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("No magic for duration calculation"));
			return 3.f;
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("No owner for cooldown calculation"));
		return 3.0f;
	}
}
