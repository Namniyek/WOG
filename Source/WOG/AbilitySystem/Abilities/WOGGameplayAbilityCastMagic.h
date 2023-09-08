// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/WOGGameplayAbilityBase.h"
#include "WOGGameplayAbilityCastMagic.generated.h"

/**
 * 
 */
class UGameplayTagContainer;

UCLASS()
class WOG_API UWOGGameplayAbilityCastMagic : public UWOGGameplayAbilityBase
{
	GENERATED_BODY()
	

public:
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Cooldown")
	FScalableFloat CooldownDuration;

	// Temp container that we will return the pointer to in GetCooldownTags().
	// This will be a union of our CooldownTags and the Cooldown GE's cooldown tags.
	UPROPERTY(Transient)
	FGameplayTagContainer TempCooldownTags;


	/** Returns all tags that are currently on cooldown */
	virtual const FGameplayTagContainer* GetCooldownTags() const override;

	/** Applies CooldownGameplayEffect to the target */
	virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;

};
