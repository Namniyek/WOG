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
	void DropWeapons();
	void SwapWeapons();

	void AttackLight();
	void AttackHeavy();
	void Block();
	void StopBlocking();

	#pragma region Cosmetic Hits
	//Handle cosmetic body hit
	void HandleCosmeticBodyHit(const FHitResult& Hit, const FVector& WeaponLocation, const AWOGBaseWeapon* InstigatorWeapon);
	FName CalculateHitDirection(const FHitResult& Hit, const FVector& WeaponLocation);
	void PlayHitReactMontage(FName Section);

	//Handle cosmetic block
	void HandleCosmeticBlock(const AWOGBaseWeapon* InstigatorWeapon);

	//Handle cosmetic weapon clash
	void HandleCosmeticWeaponClash();
	#pragma endregion


	UFUNCTION()
	void OnRep_MainWeapon();
	UFUNCTION()
	void OnRep_SecWeapon();


public:
	FVector LastHitDirection;

	UFUNCTION(Server, reliable)
	void Server_CreateMainWeapon(TSubclassOf<AWOGBaseWeapon> WeaponToCreate);
	UFUNCTION(Server, reliable)
	void Server_CreateSecondaryWeapon(TSubclassOf<AWOGBaseWeapon> WeaponToCreate);

	UFUNCTION(NetMulticast, reliable)
	void Multicast_HandleCosmeticHit(const ECosmeticHit& HitType,  const FHitResult& Hit, const FVector& WeaponLocation, const AWOGBaseWeapon* InstigatorWeapon);

	//To - do remove
	/*UFUNCTION(NetMulticast, reliable)
	void Multicast_HandleCosmeticBlock( const AWOGBaseWeapon* InstigatorWeapon);*/

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
