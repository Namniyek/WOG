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
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAGR_CombatManager> CombatManager;

	#pragma endregion

	#pragma region Handle Combat
	UFUNCTION()
	virtual void OnStartAttack();
	TArray<TObjectPtr<AActor>> HitActorsToIgnore;

	UFUNCTION()
	virtual void OnAttackHit(FHitResult Hit, UPrimitiveComponent* WeaponMesh);

	virtual void ProcessHit(FHitResult Hit, UPrimitiveComponent* WeaponMesh);
	virtual void ProcessMagicHit(const FHitResult& Hit, const struct FMagicDataTable& MagicData);

	virtual void BroadcastHit_Implementation(AActor* AgressorActor, const FHitResult& Hit, const float& DamageToApply, AActor* InstigatorWeapon);
	#pragma endregion

	#pragma region Handle Elim

	virtual void Elim(bool bPlayerLeftGame) override;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Elim(bool bPlayerLeftGame);

	UFUNCTION()
	void ElimTimerFinished();

	virtual void HandleStateElimmed(AController* InstigatedBy = nullptr) override;
	#pragma endregion
};
