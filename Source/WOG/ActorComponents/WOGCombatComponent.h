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
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


protected:
	virtual void BeginPlay() override;

	UPROPERTY(ReplicatedUsing = OnRep_MainWeapon, VisibleAnywhere, BlueprintReadOnly)
	AWOGBaseWeapon* MainWeapon;

	UPROPERTY(ReplicatedUsing = OnRep_SecWeapon, VisibleAnywhere, BlueprintReadOnly)
	AWOGBaseWeapon* SecondaryWeapon;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
	AWOGBaseWeapon* EquippedWeapon;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
	EWeaponType EquippedWeaponType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<class AWOGBaseWeapon> DefaultWeaponClass;

private:

	void CreateDefaultWeapon();

	void EquipWeapon();
	void SwapWeapons();
	void UnequipWeapon();
	void DropWeapon();

	void AttackLight();
	void AttackHeavy();
	void Block();
	void StopBlocking();



	UFUNCTION()
	void OnRep_MainWeapon();
	UFUNCTION()
	void OnRep_SecWeapon();


public:
	
	UFUNCTION(Server, reliable)
	void Server_CreateDefaultWeapon();





	
};
