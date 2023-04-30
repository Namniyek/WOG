// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WOGBaseActorComponent.h"
#include "WOG/Types/CharacterTypes.h"
#include "WOGAbilitiesComponent.generated.h"

/**
 * 
 */
class AWOGBaseAbility;

UCLASS()
class WOG_API UWOGAbilitiesComponent : public UWOGBaseActorComponent
{
	GENERATED_BODY()

public:
	UWOGAbilitiesComponent();
	friend class ABasePlayerCharacter;
	friend class AWOGDefender;
	friend class AWOGAttacker;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
protected:
	virtual void BeginPlay() override;
	TObjectPtr<ABasePlayerCharacter> OwnerPlayerCharacter;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<TSubclassOf<AWOGBaseAbility>> CurrentAbilities;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<AWOGBaseAbility> EquippedAbility;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
	EAbilityType EquippedAbilityType;

	void EquipAbility(const int32 Index);
	void UnequipAbility();

private:


public:
	void UseAbilityActionPressed();

	UFUNCTION(Server, reliable, BlueprintCallable)
	void Server_EquipAbility(const int32 Index);

	UFUNCTION(Server, reliable, BlueprintCallable)
	void Server_UnequipAbility();

	FORCEINLINE EAbilityType GetEquippedAbilityType() const { return EquippedAbilityType; }
	FORCEINLINE TObjectPtr<AWOGBaseAbility> GetEquippedAbility() const { return EquippedAbility; }

};
