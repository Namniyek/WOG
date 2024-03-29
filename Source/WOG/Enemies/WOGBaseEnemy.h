// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/WOGBaseCharacter.h"
#include "Interfaces/SpawnInterface.h"
#include "WOGBaseEnemy.generated.h"

class AWOGAttacker;
class AWOGBaseSquad;
class UWOGSpawnCosmetics;

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

	#pragma region Actor Components
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAGR_CombatManager> CombatManager;

	#pragma endregion

	#pragma region Handle Combat
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup|Combat")
	FName DefaultProjectileAttack = FName("");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup|Combat")
	FName DefaultAOEAttack = FName("");

	UPROPERTY(Replicated, BlueprintReadWrite, VisibleAnywhere)
	int32 ComboIndex;

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void DefineNextComboIndex();

	/*
	Map to store attack tags and needed tokens
	Key -> AttackTag
	Value -> Needed tokens
	*/

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup|Combat")
	TMap<FGameplayTag, int32> AttackTagsMap;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup|Combat")
	TMap<FGameplayTag, int32> RangedAttackTagsMap;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup|Combat")
	TMap<FGameplayTag, int32> CloseAttackTagsMap;

	UPROPERTY(Replicated, BlueprintReadWrite, VisibleAnywhere)
	int32 MidAttackTagIndex;
	UPROPERTY(Replicated, BlueprintReadWrite, VisibleAnywhere)
	int32 CloseAttackTagIndex;
	UPROPERTY(Replicated, BlueprintReadWrite, VisibleAnywhere)
	int32 RangedAttackTagIndex;

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void DefineNextAttackTagIndex();

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Setup|Combat")
	TSubclassOf<UGameplayEffect> DamageEffect;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Setup|Combat")
	TSubclassOf<UGameplayEffect> SecondaryDamageEffect;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
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

	virtual void ProcessHit(FHitResult Hit, UPrimitiveComponent* WeaponMesh) override;
	virtual void ProcessMagicHit(const FHitResult& Hit, const struct FMagicDataTable& MagicData) override;

	virtual void BroadcastHit_Implementation(AActor* AgressorActor, const FHitResult& Hit, const float& DamageToApply, AActor* InstigatorWeapon);

	UFUNCTION(BlueprintImplementableEvent)
	void OnCharacterElimEvent();

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void Multicast_SpawnProjectile(const FMagicDataTable& MagicData, const FName& SpawnSocket, const FRotator& SpawnRotation, bool bIsHoming);

	void SpawnProjectile(const FMagicDataTable& MagicData, const FName& SpawnSocket, const FRotator& SpawnRotation, bool bIsHoming);

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
	AActor* GetSquadCurrentTargetActor_Implementation();

	float GetAttackRangeValue_Implementation();
	float GetDefendRangeValue_Implementation();
	int32 GetComboIndex_Implementation();
	void DefineComboIndex_Implementation();
	FGameplayTag GetAttackData_Implementation(int32& TokensNeeded);
	FGameplayTag GetRangedAttackData_Implementation(int32& TokensNeeded);
	FGameplayTag GetCloseAttackData_Implementation(int32& TokensNeeded);
	FGameplayTag GetAttackDataAtIndex_Implementation(const int32& Index, int32& TokensNeeded);
	int32 GetAttackIndex_Implementation();
	void DefineAttackTagIndex_Implementation();
	void IncreaseComboIndex_Implementation();
	void ResetComboIndex_Implementation();

	#pragma endregion

	#pragma region Cosmetics
	UPROPERTY(Replicated)
	TObjectPtr<UWOGSpawnCosmetics> CosmeticsDataAsset;

	void HandleDestroyCosmetics();
	void ExecuteGameplayCueWithCosmeticsDataAsset(const FGameplayTag& CueTag);
	#pragma endregion

public:
	void HandleSpawnCosmetics();

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

	UFUNCTION(BlueprintCallable)
	void SetBaseDamage(const float& NewDamage);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void SetAttackRange(const float& NewRadius);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void SetDefendRange(const float& NewRadius);

	UFUNCTION(BlueprintPure)
	FORCEINLINE float GetAttackRange() const { return AttackRange; }

	UFUNCTION(BlueprintPure)
	FORCEINLINE float GetDefendRange() const { return DefendRange; }

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void SetDamageEffect(const TSubclassOf<UGameplayEffect>& NewDamageEffect);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void SetSecondaryDamageEffect(const TSubclassOf<UGameplayEffect>& NewDamageEffect);

	UFUNCTION(BlueprintPure)
	FORCEINLINE TSubclassOf<UGameplayEffect> GetDamageEffect() const { return DamageEffect; }

	UFUNCTION(BlueprintPure)
	FORCEINLINE TSubclassOf<UGameplayEffect> GetSecondaryDamageEffect() const { return SecondaryDamageEffect; }

	UFUNCTION(BlueprintPure)
	FORCEINLINE UWOGSpawnCosmetics* GetCosmeticsDataAsset() const { return CosmeticsDataAsset; }
	FORCEINLINE void SetCosmeticsDataAsset(UWOGSpawnCosmetics* NewAsset) { CosmeticsDataAsset = NewAsset; }
	
};
