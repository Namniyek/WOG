// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/WOGBaseCharacter.h"
#include "Interfaces/SpawnInterface.h"
#include "WOGBaseEnemy.generated.h"

UCLASS()
class WOG_API AWOGBaseEnemy : public AWOGBaseCharacter, public ISpawnInterface
{
	GENERATED_BODY()

public:
	AWOGBaseEnemy();

protected:
	virtual void BeginPlay() override;

	#pragma region Actor Components


	#pragma endregion

	#pragma region Interface Functions
	virtual void ProcessHit(FHitResult Hit, UPrimitiveComponent* WeaponMesh) override;

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

private:


public:	

};
