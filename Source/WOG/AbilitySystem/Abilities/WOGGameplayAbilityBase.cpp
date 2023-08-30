// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/WOGGameplayAbilityBase.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "Characters/WOGBaseCharacter.h"
#include "PlayerCharacter/BasePlayerCharacter.h"
#include "AbilitySystemLog.h"
#include "Types/WOGGameplayTags.h"


UWOGGameplayAbilityBase::UWOGGameplayAbilityBase()
{
	// Default to Instance Per Actor
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	// Default tags that block this ability from activating
	ActivationBlockedTags.AddTag(TAG_State_Dead);
	ActivationBlockedTags.AddTag(TAG_State_Debuff_Stagger);
	ActivationBlockedTags.AddTag(TAG_State_Debuff_Freeze);
	ActivationBlockedTags.AddTag(TAG_State_Debuff_HitReact);
	ActivationBlockedTags.AddTag(TAG_State_Debuff_Knockback);
	ActivationBlockedTags.AddTag(TAG_State_Debuff_KO);
	ActivationBlockedTags.AddTag(TAG_State_Debuff_Stun);
}

void UWOGGameplayAbilityBase::OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnAvatarSet(ActorInfo, Spec);

	if (ActivateAbilityOnGranted)
	{
		ActorInfo->AbilitySystemComponent->TryActivateAbility(Spec.Handle, false);
	}
}

void UWOGGameplayAbilityBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	UAbilitySystemComponent* AbilitySystemComponent = ActorInfo->AbilitySystemComponent.Get();
	if (!AbilitySystemComponent) return;

	FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();

	for (auto GameplayEffect : EffectsToJustApplyOnStart)
	{
		if (!GameplayEffect.Get()) continue;
		FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(GameplayEffect, 1, EffectContext);
		if (SpecHandle.IsValid())
		{
			FActiveGameplayEffectHandle ActiveGEHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			if (!ActiveGEHandle.WasSuccessfullyApplied())
			{
				UE_LOG(LogTemp, Error, TEXT("failed to apply startup effect! %s"), *GetNameSafe(GameplayEffect));
			}
		}
	}
	if (IsInstantiated())
	{
		for (auto GameplayEffect : TempEffectsDuringAbility)
		{
			if (!GameplayEffect.Get()) continue;
			FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(GameplayEffect, 1, EffectContext);
			if (SpecHandle.IsValid())
			{
				FActiveGameplayEffectHandle ActiveGEHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
				if (ActiveGEHandle.WasSuccessfullyApplied())
				{
					RemoveOnEndEffectHandles.Add(ActiveGEHandle);
				}
			}
		}
	}
}

void UWOGGameplayAbilityBase::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (IsInstantiated())
	{
		for (FActiveGameplayEffectHandle ActiveEffectHandle : RemoveOnEndEffectHandles)
		{
			if (ActiveEffectHandle.IsValid())
			{
				ActorInfo->AbilitySystemComponent->RemoveActiveGameplayEffect(ActiveEffectHandle);
			}
		}
		RemoveOnEndEffectHandles.Empty();
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

AWOGBaseCharacter* UWOGGameplayAbilityBase::GetCharacterFromActorInfo() const
{
	return Cast<AWOGBaseCharacter>(GetAvatarActorFromActorInfo());
}

ABasePlayerCharacter* UWOGGameplayAbilityBase::GetPlayerCharacterFromActorInfo() const
{
	return Cast<ABasePlayerCharacter>(GetAvatarActorFromActorInfo());
}
