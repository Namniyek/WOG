// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Types/CharacterTypes.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "WOGBlueprintLibrary.generated.h"

/**
 * 
 */
class AWOGBaseWeapon;
class AWOGBaseMagic;
class AWOGBaseConsumable;
class UWOGUIManagerComponent;
class UAbilitySystemComponent;

UCLASS()
class WOG_API UWOGBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static AWOGBaseWeapon* GetEquippedWeapon(const AActor* Owner);

	static AWOGBaseMagic* GetEquippedMagic(const AActor* Owner);

	static AWOGBaseConsumable* GetEquippedConsumable(const AActor* Owner);

	static FCharacterData GetCharacterData(AActor* Owner);

	static UWOGUIManagerComponent* GetUIManagerComponent(AController* Owner);

	static bool TryActivateAbilityByTagWithData(AActor* Target, const FGameplayTag& ActivationTag, FGameplayEventData AbilityData);

private:
	//Can return nullptr 
	UFUNCTION(BlueprintCallable, BlueprintPure, DisplayName = "Get Equipped Weapon", Category = "WOG Library")
	static AWOGBaseWeapon* K2_GetEquippedWeapon(AActor* Owner)
	{
		return GetEquippedWeapon(Owner);
	};

	//Can return nullptr 
	UFUNCTION(BlueprintCallable, BlueprintPure, DisplayName = "Get Equipped Magic", Category = "WOG Library")
	static AWOGBaseMagic* K2_GetEquippedMagic(AActor* Owner)
	{
		return GetEquippedMagic(Owner);
	};

	//Can return nullptr 
	UFUNCTION(BlueprintCallable, BlueprintPure, DisplayName = "Get Equipped Consumable", Category = "WOG Library")
	static AWOGBaseConsumable* K2_GetEquippedConsumable(AActor* Owner)
	{
		return GetEquippedConsumable(Owner);
	};

	//Can return nullptr
	UFUNCTION(BlueprintCallable, BlueprintPure, DisplayName = "Get Character Data", Category = "WOG Library")
	static FCharacterData K2_GetCharacterData(AActor* Owner)
	{
		return GetCharacterData(Owner);
	};

	//Can return nullptr
	UFUNCTION(BlueprintCallable, BlueprintPure, DisplayName = "Get UI Manager Component", Category = "WOG Library")
	static UWOGUIManagerComponent* K2_UIManagerComponent(AController* Owner)
	{
		return GetUIManagerComponent(Owner);
	};

	//Tries to activate an ability by a tag passing custom data into the ability
	UFUNCTION(BlueprintCallable, BlueprintPure, DisplayName = "Try Activate Ability by Tag with Data", Category = "WOG Library")
		static bool K2_UTryActivateAbilityByTagWithData(AActor* Target, const FGameplayTag& ActivationTag, FGameplayEventData AbilityData)
	{
		return TryActivateAbilityByTagWithData(Target, ActivationTag, AbilityData);
	};

};
