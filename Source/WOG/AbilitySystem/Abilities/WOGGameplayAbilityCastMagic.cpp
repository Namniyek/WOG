// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/WOGGameplayAbilityCastMagic.h"
#include "Magic/WOGBaseMagic.h"
#include "Libraries/WOGBlueprintLibrary.h"
#include "Data/WOGGameplayTags.h"

const FGameplayTagContainer* UWOGGameplayAbilityCastMagic::GetCooldownTags() const
{
	if (!GetAvatarActorFromActorInfo())
	{
		UE_LOG(LogTemp, Error, TEXT("No AvatarActor during GetCooldownTags()"));
		return nullptr;
	}
	TObjectPtr<AWOGBaseMagic> Magic = UWOGBlueprintLibrary::GetEquippedMagic(GetAvatarActorFromActorInfo());
	if (!Magic)
	{
		UE_LOG(LogTemp, Error, TEXT("No Equipped Magic during GetCooldownTags()"));
		return nullptr;
	}

	FGameplayTagContainer* MutableTags = const_cast<FGameplayTagContainer*>(&TempCooldownTags);
	MutableTags->Reset(); // MutableTags writes to the TempCooldownTags on the CDO so clear it in case the ability cooldown tags change (moved to a different slot)
	const FGameplayTagContainer* ParentTags = Super::GetCooldownTags();
	if (ParentTags)
	{
		MutableTags->AppendTags(*ParentTags);
	}
	MutableTags->AddTag(Magic->GetMagicData().CooldownTag);
	return MutableTags;
}

void UWOGGameplayAbilityCastMagic::ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	if (!GetAvatarActorFromActorInfo())
	{
		UE_LOG(LogTemp, Error, TEXT("No AvatarActor during ApplyCooldown()"));
		return;
	}
	TObjectPtr<AWOGBaseMagic> Magic = UWOGBlueprintLibrary::GetEquippedMagic(GetAvatarActorFromActorInfo());
	if (!Magic)
	{
		UE_LOG(LogTemp, Error, TEXT("No Equipped Magic during ApplyCooldown()"));
		return;
	}

	UGameplayEffect* CooldownGE = GetCooldownGameplayEffect();
	if (CooldownGE)
	{
		FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(CooldownGE->GetClass(), GetAbilityLevel());
		SpecHandle.Data.Get()->DynamicGrantedTags.AddTag(Magic->GetMagicData().CooldownTag);
		SpecHandle.Data.Get()->SetSetByCallerMagnitude(TAG_Data_Cooldown, Magic->GetMagicData().Cooldown);
		ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
	}
}
