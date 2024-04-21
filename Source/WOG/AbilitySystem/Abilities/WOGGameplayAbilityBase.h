// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
//#include "Types/CharacterTypes.h"
#include "Data/WOGDataTypes.h"
#include "WOGGameplayAbilityBase.generated.h"

/**
 * 
 */
 /** Used to notify ability state tasks that a state is being ended */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGameplayAbilityCostChecked, bool, bIsEnough, FGameplayAttribute, Attribute);

UCLASS()
class WOG_API UWOGGameplayAbilityBase : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UWOGGameplayAbilityBase(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(BlueprintReadOnly, Category = "Ability")
	FGameplayEventData ActivationDataByTag;

	// Abilities with this set will automatically activate when the input is pressed
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Ability")
	EWOGAbilityInputID AbilityInputID = EWOGAbilityInputID::None;

	// Value to associate an ability with an slot without tying it to an automatically activated input.
	// Passive abilities won't be tied to an input so we need a way to generically associate abilities with slots.
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Ability")
	EWOGAbilityInputID AbilityID = EWOGAbilityInputID::None;

	// Tells an ability to activate immediately when its granted. Used for passive abilities and abilities forced on others.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ability")
	bool ActivateAbilityOnGranted = false;

	// If an ability is marked as 'ActivateAbilityOnGranted', activate them immediately when given here
	// Epic's comment: Projects may want to initiate passives or do other "BeginPlay" type of logic here.
	virtual void OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;

	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	/** Checks cost. returns true if we can pay for the ability. False if not */
	virtual bool CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	
protected:
	bool bHasBlueprintActivateByTag;

	//Effects that are apllied at start of the ability and removed at the end
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	TArray <TSubclassOf<UGameplayEffect>> TempEffectsDuringAbility;

	//Effects that are apllied at start of the ability
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	TArray <TSubclassOf<UGameplayEffect>> EffectsToJustApplyOnStart;

	TArray<FActiveGameplayEffectHandle> RemoveOnEndEffectHandles;
	UPROPERTY(BlueprintReadOnly)
	TArray<FActiveGameplayEffectHandle> ActiveEffectHandles;

	UFUNCTION(BlueprintPure)
	class AWOGBaseCharacter* GetCharacterFromActorInfo() const;

	UFUNCTION(BlueprintPure)
	class ABasePlayerCharacter* GetPlayerCharacterFromActorInfo() const;

	UPROPERTY(BlueprintAssignable)
	FOnGameplayAbilityCostChecked OnCostCheckedDelegate;

	UFUNCTION()
	void OnCostChecked(bool bIsEnough, FGameplayAttribute Attribute);

	UFUNCTION(BlueprintCallable)
	void ExecuteGameplayCueLocal(FGameplayTag CueTag, const FGameplayCueParameters& Parameters = FGameplayCueParameters());

	UFUNCTION(BlueprintCallable)
	void RemoveGameplayCueLocal(FGameplayTag CueTag, const FGameplayCueParameters& Parameters = FGameplayCueParameters());

	UFUNCTION(BlueprintImplementableEvent, Category = Ability, DisplayName = "ActivateAbilityByTag", meta = (ScriptName = "ActivateAbilityByTag"))
	void K2_ActivateAbilityByTag(const FGameplayEventData& EventData);
};
