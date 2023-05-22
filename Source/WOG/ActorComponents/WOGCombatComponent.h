// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WOGBaseActorComponent.h"
#include "WOG/Types/CharacterTypes.h"
#include "WOGCombatComponent.generated.h"


class AWOGBaseWeapon;
/**
 * 
 */
UCLASS()
class WOG_API UWOGCombatComponent : public UWOGBaseActorComponent
{
	GENERATED_BODY()
	
public:
	UWOGCombatComponent();
	friend class ABasePlayerCharacter;
	friend class AWOGDefender;
	friend class AWOGAttacker;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


protected:
	virtual void BeginPlay() override;
	TObjectPtr<ABasePlayerCharacter> OwnerPlayerCharacter;

	UPROPERTY(ReplicatedUsing = OnRep_MainWeapon, VisibleAnywhere, BlueprintReadOnly)
	AWOGBaseWeapon* MainWeapon;

	UPROPERTY(ReplicatedUsing = OnRep_SecWeapon, VisibleAnywhere, BlueprintReadOnly)
	AWOGBaseWeapon* SecondaryWeapon;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
	AWOGBaseWeapon* EquippedWeapon;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
	EWeaponType EquippedWeaponType;

private:

	TSubclassOf<class AWOGBaseWeapon> DefaultWeaponClass;
	void CreateMainWeapon(TSubclassOf<AWOGBaseWeapon> WeaponToCreate);
	void CreateSecondaryWeapon(TSubclassOf<AWOGBaseWeapon> WeaponToCreate);

	void EquipMainWeapon();
	void EquipSecondaryWeapon();
	void UnequipMainWeapon();
	void UnequipSecondaryWeapon();
	void StoreEquippedWeapon();
	void SwapWeapons();

	UFUNCTION()
	void OnRep_MainWeapon();
	UFUNCTION()
	void OnRep_SecWeapon();

public:
	UFUNCTION(Server, reliable)
	void Server_CreateMainWeapon(TSubclassOf<AWOGBaseWeapon> WeaponToCreate);
	UFUNCTION(Server, reliable)
	void Server_CreateSecondaryWeapon(TSubclassOf<AWOGBaseWeapon> WeaponToCreate);

	FORCEINLINE void SetDefaultWeaponClass(TSubclassOf<AWOGBaseWeapon> NewDefaultWeapon) { DefaultWeaponClass = NewDefaultWeapon; }
	void SetEquippedWeapon(AWOGBaseWeapon* NewEquippedWeapon);
	FORCEINLINE AWOGBaseWeapon* GetEquippedWeapon() const { return EquippedWeapon; }
	FORCEINLINE AWOGBaseWeapon* GetMainWeapon() const { return MainWeapon; }
	FORCEINLINE AWOGBaseWeapon* GetSecondaryWeapon() const { return SecondaryWeapon; }
	FORCEINLINE TSubclassOf<AWOGBaseWeapon> GetDefaultWeaponClass() const { return DefaultWeaponClass; }
	
	/*
	* TEST SECTION
	*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<class AWOGBaseWeapon> SecondaryWeaponClass;

};
