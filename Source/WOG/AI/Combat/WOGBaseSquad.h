// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Data/WOGDataTypes.h"
#include "WOGBaseSquad.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTargetDestroyedDelegate, AActor*, Destroyer);

UCLASS()
class WOG_API AWOGBaseSquad : public AActor
{
	GENERATED_BODY()
	
public:	
	friend class UWOGEnemyOrderComponent;
	AWOGBaseSquad();
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(Replicated, BlueprintReadOnly, VisibleAnywhere)
	EEnemyOrder CurrentSquadOrder;

	UPROPERTY(Replicated, BlueprintReadOnly, VisibleAnywhere)
	TObjectPtr<AActor> CurrentTargetActor;

	UPROPERTY(Replicated, BlueprintReadOnly, VisibleAnywhere)
	FVector_NetQuantize CurrentTargetLocation;

	UPROPERTY(Replicated, BlueprintReadOnly, VisibleAnywhere)
	EEnemySquadType SquadType;

	#pragma region Slot Components

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<USceneComponent> RootComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<USceneComponent> Slot_0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<USceneComponent> Slot_1;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<USceneComponent> Slot_2;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<USceneComponent> Slot_3;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<USceneComponent> Slot_4;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<USceneComponent> Slot_5;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<USceneComponent> Slot_6;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<USceneComponent> Slot_7;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<USceneComponent> Slot_8;
	#pragma endregion

private:
	void SetEnemyStateOnSquad(const EEnemyState& NewState);
	void CheckIsSquadEmpty();
	void DeregisterSquad();

	AActor* FindRandomTarget();
	AActor* GetClosestActor(TArray<AActor*> InArray);
	
	UFUNCTION()
	void OnCurrentTargetDestroyed(AActor* Destroyer);

public:	
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void SendOrder(const EEnemyOrder& NewOrder, const FTransform& TargetTansform = FTransform(), AActor* TargetActor = nullptr);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	FORCEINLINE void SetSquadType(EEnemySquadType& NewType) { SquadType = NewType; };

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TArray<USceneComponent*> SlotComponentsArray;

	UPROPERTY(Replicated, BlueprintReadOnly, EditAnywhere)
	TArray<FEnemyCombatSlot> SquadSlots;

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void SetCurrentSquadOrder(const EEnemyOrder& NewOrder);

	UFUNCTION(BlueprintPure)
	FORCEINLINE EEnemyOrder GetCurrentSquadOrder() const { return CurrentSquadOrder; }

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void SetCurrentTargetActor(AActor* NewTarget);

	UFUNCTION(BlueprintPure)
	FORCEINLINE AActor* GetCurrentTargetActor() const { return CurrentTargetActor; }

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void SetCurrentTargetLocation(const FVector_NetQuantize& NewTarget);

	UFUNCTION(BlueprintPure)
	FORCEINLINE FVector_NetQuantize GetCurrentTargetLocation() const { return CurrentTargetLocation; }

	UFUNCTION()
	void DeregisterDeadSquadMember(AWOGBaseEnemy* DeadEnemy);

	UPROPERTY(Replicated, BlueprintReadOnly)
	FText SquadName = FText();

	UPROPERTY(Replicated, BlueprintReadOnly)
	TObjectPtr<UTexture2D> SquadIcon = nullptr;
};
