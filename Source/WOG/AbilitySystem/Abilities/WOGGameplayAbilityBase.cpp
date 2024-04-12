// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/WOGGameplayAbilityBase.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "Characters/WOGBaseCharacter.h"
#include "PlayerCharacter/BasePlayerCharacter.h"
#include "AbilitySystemGlobals.h"
#include "Data/WOGGameplayTags.h"
#include "PlayerController/WOGPlayerController.h"
#include "ActorComponents/WOGAbilitySystemComponent.h"
#include "Subsystems/WOGUIManagerSubsystem.h"

UWOGGameplayAbilityBase::UWOGGameplayAbilityBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
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


	OnCostCheckedDelegate.AddDynamic(this, &ThisClass::OnCostChecked);

	auto ImplementedInBlueprint = [](const UFunction* Func) -> bool
	{
		return Func && ensure(Func->GetOuter())
			&& Func->GetOuter()->IsA(UBlueprintGeneratedClass::StaticClass());
	};
	{
		static FName FuncName = FName(TEXT("K2_ActivateAbilityByTag"));
		UFunction* ActivateFunction = GetClass()->FindFunctionByName(FuncName);
		bHasBlueprintActivateByTag = ImplementedInBlueprint(ActivateFunction);
	}
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
			if (ActiveGEHandle.WasSuccessfullyApplied())
			{
				ActiveEffectHandles.Add(ActiveGEHandle);
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
					ActiveEffectHandles.Add(ActiveGEHandle);
				}
			}
		}
	}

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (bHasBlueprintActivateByTag && GetCharacterFromActorInfo())
	{
		K2_ActivateAbilityByTag(GetCharacterFromActorInfo()->AbilityActivationPayload);
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

	ActiveEffectHandles.Empty();

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

bool UWOGGameplayAbilityBase::CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	/*
	*Same code as the parent version, with the exception of the broadcasting of the OnCastChecked delegate before return.
	*"Super::" not needed
	*/

	FGameplayAttribute CostAttribute;

	UGameplayEffect* CostGE = GetCostGameplayEffect();
	if (CostGE)
	{
		/*
		*Calculations to get the affected attribute
		*/
		FGameplayEffectSpec	Spec(CostGE, MakeEffectContext(Handle, ActorInfo), GetAbilityLevel(Handle, ActorInfo));
		Spec.CalculateModifierMagnitudes();
		const FGameplayModifierInfo& ModDef = Spec.Def->Modifiers[0];
		CostAttribute = ModDef.Attribute;

		UAbilitySystemComponent* const AbilitySystemComponent = ActorInfo->AbilitySystemComponent.Get();
		check(AbilitySystemComponent != nullptr);
		if (!AbilitySystemComponent->CanApplyAttributeModifiers(CostGE, GetAbilityLevel(Handle, ActorInfo), MakeEffectContext(Handle, ActorInfo)))
		{
			const FGameplayTag& CostTag = UAbilitySystemGlobals::Get().ActivateFailCostTag;

			if (OptionalRelevantTags && CostTag.IsValid())
			{
				OptionalRelevantTags->AddTag(CostTag);
			}

			if (CostAttribute.GetName() == FString("Adrenaline") || CostAttribute.GetName() == FString("Mana"))
			{
				OnCostCheckedDelegate.Broadcast(false, CostAttribute);
			}
			return false;
		}

		if (CostAttribute.GetName() == FString("Adrenaline") || CostAttribute.GetName() == FString("Mana"))
		{
			OnCostCheckedDelegate.Broadcast(true, CostAttribute);
		}
		return true;
	}

	if (CostAttribute.GetName() == FString("Adrenaline") || CostAttribute.GetName() == FString("Mana"))
	{
		OnCostCheckedDelegate.Broadcast(true, CostAttribute);
	}
	return true;
}

AWOGBaseCharacter* UWOGGameplayAbilityBase::GetCharacterFromActorInfo() const
{
	return Cast<AWOGBaseCharacter>(GetAvatarActorFromActorInfo());
}

ABasePlayerCharacter* UWOGGameplayAbilityBase::GetPlayerCharacterFromActorInfo() const
{
	return Cast<ABasePlayerCharacter>(GetAvatarActorFromActorInfo());
}

void UWOGGameplayAbilityBase::OnCostChecked(bool bIsEnough, FGameplayAttribute Attribute)
{
	if (!GetCharacterFromActorInfo() || !GetCharacterFromActorInfo()->Controller || !GetCharacterFromActorInfo()->IsLocallyControlled())
	{
		return;
	}
	
	if (!bIsEnough)
	{
		TObjectPtr<AWOGPlayerController> OwnerPC = Cast<AWOGPlayerController>(GetCharacterFromActorInfo()->Controller);
		if (!OwnerPC) return;
		TObjectPtr<UWOGUIManagerSubsystem> UIManager = ULocalPlayer::GetSubsystem<UWOGUIManagerSubsystem>(OwnerPC->GetLocalPlayer());
		if (UIManager)
		{
			UIManager->CreateResourceWarningWidget(Attribute.GetName());
		}
	}
}

void UWOGGameplayAbilityBase::ExecuteGameplayCueLocal(FGameplayTag CueTag, const FGameplayCueParameters& Parameters)
{
	TObjectPtr<UWOGAbilitySystemComponent> ASC = Cast<UWOGAbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo());
	if (ASC)
	{
		ASC->ExecuteGameplayCueLocal(CueTag, Parameters);
		UE_LOG(LogTemp, Warning, TEXT("Local cue executed %s"), *CueTag.ToString());
	}
}

void UWOGGameplayAbilityBase::RemoveGameplayCueLocal(FGameplayTag CueTag, const FGameplayCueParameters& Parameters)
{
	TObjectPtr<UWOGAbilitySystemComponent> ASC = CastChecked<UWOGAbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo());
	ASC->RemoveGameplayCueLocal(CueTag, Parameters);
}
