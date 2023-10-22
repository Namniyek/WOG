// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/WOGGameplayAbilityUseWeapon.h"
#include "Libraries/WOGBlueprintLibrary.h"
#include "Types/WOGGameplayTags.h"
#include "Weapons/WOGBaseWeapon.h"

const FGameplayTagContainer* UWOGGameplayAbilityUseWeapon::GetCooldownTags() const
{
	if (!GetAvatarActorFromActorInfo())
	{
		UE_LOG(LogTemp, Error, TEXT("No AvatarActor during GetCooldownTags()"));
		return nullptr;
	}
	TObjectPtr<AWOGBaseWeapon> Weapon = UWOGBlueprintLibrary::GetEquippedWeapon(GetAvatarActorFromActorInfo());
	if (!Weapon)
	{
		UE_LOG(LogTemp, Error, TEXT("No Equipped Weapon during GetCooldownTags()"));
		return nullptr;
	}

	FGameplayTagContainer* MutableTags = const_cast<FGameplayTagContainer*>(&TempCooldownTags);
	MutableTags->Reset(); // MutableTags writes to the TempCooldownTags on the CDO so clear it in case the ability cooldown tags change (moved to a different slot)
	const FGameplayTagContainer* ParentTags = Super::GetCooldownTags();
	if (ParentTags)
	{
		MutableTags->AppendTags(*ParentTags);
	}
	MutableTags->AddTag(Weapon->GetWeaponData().CooldownTag);
	return MutableTags;
}

void UWOGGameplayAbilityUseWeapon::ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	if (!GetAvatarActorFromActorInfo())
	{
		UE_LOG(LogTemp, Error, TEXT("No AvatarActor during ApplyCooldown()"));
		return;
	}
	TObjectPtr<AWOGBaseWeapon> Weapon = UWOGBlueprintLibrary::GetEquippedWeapon(GetAvatarActorFromActorInfo());
	if (!Weapon)
	{
		UE_LOG(LogTemp, Error, TEXT("No Equipped Weapon during GetCooldownTags()"));
		return;
	}

	UGameplayEffect* CooldownGE = GetCooldownGameplayEffect();
	if (CooldownGE)
	{
		FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(CooldownGE->GetClass(), GetAbilityLevel());
		SpecHandle.Data.Get()->DynamicGrantedTags.AddTag(Weapon->GetWeaponData().CooldownTag);
		SpecHandle.Data.Get()->SetSetByCallerMagnitude(TAG_Data_Cooldown, Weapon->GetWeaponData().Cooldown);
		ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
	}
}
