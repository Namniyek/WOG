// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/WOGDataTypes.h"
#include "GameFramework/Actor.h"
#include "Interfaces/BuildingInterface.h"
#include "Interfaces/TargetInterface.h"
#include "WOGBaseTarget.generated.h"

class UGeometryCollectionComponent;
class UGeometryCollection;
class AWOGDayNPCSpawner;

UCLASS()
class WOG_API AWOGBaseTarget : public AActor, public IBuildingInterface, public ITargetInterface
{
	GENERATED_BODY()
	
public:	
	AWOGBaseTarget();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<AWOGDayNPCSpawner> ConnectedSpawner;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UGeometryCollectionComponent> GeometryCollectionComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> RootMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float DestroyDelay;

	/** Sets a new Rest Collection. */
	UFUNCTION(BlueprintCallable, Category = "ChaosPhysics")
	void UpdateRestCollection(const UGeometryCollection* RestCollectionIn);

	virtual void DealDamage_Implementation(const float& Damage, const AActor* Agressor) override;

	UFUNCTION()
	void HandleDamage(const float& Damage, const AActor* Agressor);

	UFUNCTION(BlueprintImplementableEvent)
	void HandleChaosDestruction();

	UFUNCTION(NetMulticast, unreliable)
	void Multicast_ForceTargetOff(const AActor* Agressor);

	UFUNCTION(BlueprintImplementableEvent)
	void ShowHealthBar();

	UFUNCTION(BlueprintImplementableEvent)
	void HideHealthBar();

	FTimerHandle HealthBarTimerHandle;
	void HandleHealthBar(bool NewVisible);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float MaxHealth;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, ReplicatedUsing = "OnRep_Health")
	float Health;

	UFUNCTION()
	void OnRep_Health();

	UFUNCTION()
	void HandleDestruction();

	void DestroyTarget();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_HandleDoorOpening(bool bIsOpen);

	UFUNCTION(BlueprintImplementableEvent)
	void HandleDoorOpening(bool bIsOpen);

	UFUNCTION()
	void TimeOfDayChanged(ETimeOfDay TOD);

	UFUNCTION()
	void KeyTimeHit(int32 CurrentTime);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 TargetScore;

	void BroadcastDestructionToGameState();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FVector3DWithWidget> MeleeSlots;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FVector3DWithWidget> RangedSlots;

	#pragma region Interface functions
	bool IsTargetable_Implementation(AActor* TargeterActor) const;
	FVector GetMeleeAttackSlot_Implementation(const int32& SlotIndex) const;
	FVector GetRangedAttackSlot_Implementation(const int32& SlotIndex) const;
	#pragma endregion

public:	

	UFUNCTION(BlueprintPure)
	FORCEINLINE int32 GetTargetScore() const { return TargetScore; }
};
