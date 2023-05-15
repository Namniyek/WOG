// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WOG/Characters/WOGBaseCharacter.h"
#include "WOG/Interfaces/SpawnInterface.h"
#include "WOGBaseEnemy.generated.h"

UCLASS()
class WOG_API AWOGBaseEnemy : public AWOGBaseCharacter, public ISpawnInterface
{
	GENERATED_BODY()

public:
	AWOGBaseEnemy();

protected:
	virtual void BeginPlay() override;
	virtual void OnHealthAttributeChanged(const FOnAttributeChangeData& Data);

	#pragma region Actor Components

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<class ULockOnTargetComponent> LockOnTarget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<class UTargetingHelperComponent> TargetAttractor;

	#pragma endregion

	#pragma region Interface Functions
	virtual void BroadcastHit_Implementation(AActor* AgressorActor, const FHitResult& Hit, const float& DamageToApply, AActor* InstigatorWeapon) override;

	#pragma endregion

	virtual void HandleStateElimmed(AController* InstigatedBy = nullptr) override;

	#pragma region Handle Damage

	virtual void Elim(bool bPlayerLeftGame) override;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Elim(bool bPlayerLeftGame);

	UFUNCTION()
	void ElimTimerFinished();

	#pragma endregion

	#pragma region Cosmetic Hits
	//Handle cosmetic body hit
	virtual void HandleCosmeticBodyHit(const FHitResult& Hit, const FVector& WeaponLocation, const class AWOGBaseWeapon* InstigatorWeapon) override;
	virtual void PlayHitReactMontage(FName Section) override;

	//Handle cosmetic block
	virtual void HandleCosmeticBlock(const AWOGBaseWeapon* InstigatorWeapon) override;

	//Handle cosmetic weapon clash
	virtual void HandleCosmeticWeaponClash() override;
	#pragma endregion

private:


public:	
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
