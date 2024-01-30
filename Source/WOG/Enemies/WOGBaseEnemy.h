// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/WOGBaseCharacter.h"
#include "Interfaces/SpawnInterface.h"
#include "WOGBaseEnemy.generated.h"

class AWOGAttacker;
class AWOGBaseSquad;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEnemyStateChanged);

UCLASS()
class WOG_API AWOGBaseEnemy : public AWOGBaseCharacter, public ISpawnInterface
{
	GENERATED_BODY()

public:
	AWOGBaseEnemy();
	virtual void OnConstruction(const FTransform& Transform) override;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<AWOGAttacker> OwnerAttacker;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<AWOGBaseSquad> OwnerSquad;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
	int32 SquadUnitIndex;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> StaticMeshWeapon;

	#pragma region Actor Components
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAGR_CombatManager> CombatManager;

	#pragma endregion

	#pragma region Handle Combat
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TSubclassOf<UGameplayEffect> DamageEffect;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	float BaseDamage = 10.f;

	UPROPERTY(Replicated, VisibleAnywhere)
	float AttackRange;

	UPROPERTY(Replicated, VisibleAnywhere)
	float DefendRange;

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

	virtual void Destroyed() override;
	#pragma endregion

	UPROPERTY(Replicated, VisibleAnywhere)
	EEnemyState CurrentEnemyState;

	UPROPERTY(BlueprintAssignable)
	FOnEnemyStateChanged OnEnemyStateChangedDelegate;

	#pragma region Interface functions

	AWOGBaseSquad* GetEnemyOwnerSquad_Implementation();

	int32 GetEnemySquadUnitIndex_Implementation();

	float GetAttackRangeValue_Implementation();
	float GetDefendRangeValue_Implementation();
	#pragma endregion

	#pragma region Animation
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup|Animations")
	TObjectPtr<UAnimMontage> AttackMontage;
	#pragma endregion

public:
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void SetOwnerAttacker(AWOGAttacker* NewOwner);

	UFUNCTION(BlueprintPure)
	FORCEINLINE AWOGAttacker* GetOwnerAttacker() const { return OwnerAttacker; }

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void SetOwnerSquad(AWOGBaseSquad* NewOwnerSquad);

	UFUNCTION(BlueprintPure)
	FORCEINLINE AWOGBaseSquad* GetOwnerSquad() const { return OwnerSquad; }

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void SetSquadUnitIndex(const int32& NewIndex);

	UFUNCTION(BlueprintPure)
	FORCEINLINE int32 GetSquadUnitIndex() const { return SquadUnitIndex; }

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void SetCurrentEnemyState(const EEnemyState& NewState);

	UFUNCTION(BlueprintPure)
	FORCEINLINE EEnemyState GetCurrentEnemyState() const { return CurrentEnemyState; }

	void SetBaseDamage(const float& NewDamage);
	void SetAttackMontage(UAnimMontage* NewMontage);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void SetAttackRange(const float& NewRadius);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void SetDefendRange(const float& NewRadius);

	UFUNCTION(BlueprintPure)
	FORCEINLINE float GetAttackRange() const { return AttackRange; }

	UFUNCTION(BlueprintPure)
	FORCEINLINE float GetDefendRange() const { return DefendRange; }

	UFUNCTION()
	void SetDamageEffect(const TSubclassOf<UGameplayEffect>& NewDamageEffect);
	UFUNCTION(BlueprintPure)
	FORCEINLINE TSubclassOf<UGameplayEffect> GetDamageEffect() const { return DamageEffect; }
};
