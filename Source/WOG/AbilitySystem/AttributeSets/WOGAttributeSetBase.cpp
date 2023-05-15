// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/AttributeSets/WOGAttributeSetBase.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"


void UWOGAttributeSetBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION_NOTIFY(UWOGAttributeSetBase, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UWOGAttributeSetBase, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UWOGAttributeSetBase, Mana, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UWOGAttributeSetBase, MaxMana, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UWOGAttributeSetBase, Adrenaline, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UWOGAttributeSetBase, MaxAdrenaline, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UWOGAttributeSetBase, Stamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UWOGAttributeSetBase, MaxStamina, COND_None, REPNOTIFY_Always);
}

void UWOGAttributeSetBase::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));
	}
	if (Data.EvaluatedData.Attribute == GetManaAttribute())
	{
		SetMana(FMath::Clamp(GetMana(), 0.0f, GetMaxMana()));
	}
	if (Data.EvaluatedData.Attribute == GetAdrenalineAttribute())
	{
		SetAdrenaline(FMath::Clamp(GetAdrenaline(), 0.0f, GetMaxAdrenaline()));
	}
	if (Data.EvaluatedData.Attribute == GetStaminaAttribute())
	{
		SetStamina(FMath::Clamp(GetStamina(), 0.0f, GetMaxStamina()));
	}
}

void UWOGAttributeSetBase::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UWOGAttributeSetBase, Health, OldHealth);
}

void UWOGAttributeSetBase::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UWOGAttributeSetBase, MaxHealth, OldMaxHealth);
}

void UWOGAttributeSetBase::OnRep_Mana(const FGameplayAttributeData& OldMana)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UWOGAttributeSetBase, Mana, OldMana);
}

void UWOGAttributeSetBase::OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UWOGAttributeSetBase, MaxMana, OldMaxMana);
}

void UWOGAttributeSetBase::OnRep_Adrenaline(const FGameplayAttributeData& OldAdrenaline)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UWOGAttributeSetBase, Adrenaline, OldAdrenaline);
}

void UWOGAttributeSetBase::OnRep_MaxAdrenaline(const FGameplayAttributeData& OldMaxAdrenaline)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UWOGAttributeSetBase, MaxAdrenaline, OldMaxAdrenaline);
}

void UWOGAttributeSetBase::OnRep_Stamina(const FGameplayAttributeData& OldStamina)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UWOGAttributeSetBase, Stamina, OldStamina);
}

void UWOGAttributeSetBase::OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UWOGAttributeSetBase, MaxStamina, OldMaxStamina);
}
