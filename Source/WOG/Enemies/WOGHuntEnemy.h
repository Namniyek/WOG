// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemies/WOGBaseEnemy.h"
#include "WOGHuntEnemy.generated.h"

/**
 * 
 */
class USphereComponent;

UCLASS()
class WOG_API AWOGHuntEnemy : public AWOGBaseEnemy
{
	GENERATED_BODY()
	
public:
	AWOGHuntEnemy();
	virtual void PostInitializeComponents();
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;

protected:

	virtual void BeginPlay() override;

	#pragma region ActorComponents
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	USphereComponent* AgroSphere;
	#pragma endregion

	#pragma region Setup Variables
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup")
	TArray<FName> MinionNames;

	UPROPERTY(ReplicatedUsing = OnRep_MinionLevel, VisibleAnywhere, BlueprintReadOnly)
	int32 MinionLevel;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup")
	TObjectPtr<UDataTable> MinionDataTable;
	#pragma endregion

	#pragma region HandleCombat
	
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Replicated)
	TObjectPtr<AActor> CurrentTarget = nullptr;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Replicated)
	TArray<TObjectPtr<AActor>> CurrentTargetArray = {};

	/*
	Map to store attack tags and needed tokens
	Key -> AttackTag
	Value -> Needed tokens
	*/

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup|Combat")
	TMap<FGameplayTag, int32> AdditionalMidAttackTagsMap;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup|Combat")
	TMap<FGameplayTag, int32> AdditionalRangedAttackTagsMap;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup|Combat")
	TMap<FGameplayTag, int32> AdditionalCloseAttackTagsMap;

	#pragma endregion

	#pragma region Interface Functions
	AActor* GetSquadCurrentTargetActor_Implementation();
	void SetCurrentTarget_Implementation(AActor* NewTarget);
	AActor* FindClosestPlayerTarget_Implementation();
	AActor* GetIdleSpotLocation_Implementation();
	#pragma endregion

	UFUNCTION()
	void OnAgroOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnAgroEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void OnAttributeChangedCallback(FGameplayAttribute ChangedAttribute, float NewValue, float MaxValue);

	UFUNCTION()
	void OnRep_MinionLevel(const int32& OldLevel);

private:
	virtual void InitData();

	AActor* FindRandomClosestPlayer();
	AActor* GetClosestActor(TArray<AActor*> InArray);

public:
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void SetMinionLevel(const int32& NewLevel);

	void ClearAbilties();

	void FindNewTarget();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<AActor> IdleSpotLocation;

	void MergeAttackTagMaps();
};
