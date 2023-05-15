// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "WOGAttributeSetBase.generated.h"

/**
 * 
 */

 // Uses macros from AttributeSet.h
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class WOG_API UWOGAttributeSetBase : public UAttributeSet
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_Health)
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UWOGAttributeSetBase, Health)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UWOGAttributeSetBase, MaxHealth)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_Mana)
	FGameplayAttributeData Mana;
	ATTRIBUTE_ACCESSORS(UWOGAttributeSetBase, Mana)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_MaxMana)
	FGameplayAttributeData MaxMana;
	ATTRIBUTE_ACCESSORS(UWOGAttributeSetBase, MaxMana)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_Adrenaline)
	FGameplayAttributeData Adrenaline;
	ATTRIBUTE_ACCESSORS(UWOGAttributeSetBase, Adrenaline)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_MaxAdrenaline)
	FGameplayAttributeData MaxAdrenaline;
	ATTRIBUTE_ACCESSORS(UWOGAttributeSetBase, MaxAdrenaline)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_Stamina)
	FGameplayAttributeData Stamina;
	ATTRIBUTE_ACCESSORS(UWOGAttributeSetBase, Stamina)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_MaxStamina)
	FGameplayAttributeData MaxStamina;
	ATTRIBUTE_ACCESSORS(UWOGAttributeSetBase, MaxStamina)

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;

	UFUNCTION()
	virtual void OnRep_Health(const FGameplayAttributeData& OldHealth);

	UFUNCTION()
	virtual void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth);

	UFUNCTION()
	virtual void OnRep_Mana(const FGameplayAttributeData& OldMana);

	UFUNCTION()
	virtual void OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana);

	UFUNCTION()
	virtual void OnRep_Adrenaline(const FGameplayAttributeData& OldAdrenaline);

	UFUNCTION()
	virtual void OnRep_MaxAdrenaline(const FGameplayAttributeData& OldMaxAdrenaline);

	UFUNCTION()
	virtual void OnRep_Stamina(const FGameplayAttributeData& OldStamina);

	UFUNCTION()
	virtual void OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina);
	
};
