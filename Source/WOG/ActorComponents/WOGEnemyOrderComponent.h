// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActorComponents/WOGBaseActorComponent.h"
#include "Data/WOGDataTypes.h"
#include "WOGEnemyOrderComponent.generated.h"

/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSquadUpdatedDelegate, bool, bSquadAdded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCurrentActiveSquadUpdatedDelegate, int32, CurrentActiveSquadIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOrderSentDelegate, int32, OrderIndex);

class AWOGBaseSquad;
class AWOGAttacker;

UCLASS()
class WOG_API UWOGEnemyOrderComponent : public UWOGBaseActorComponent
{
	GENERATED_BODY()

public:
	UWOGEnemyOrderComponent();
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
	TArray<AWOGBaseSquad*> CurrentSquads;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<AWOGAttacker> OwnerAttacker;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<AWOGBaseSquad> CurrentlySelectedSquad;

	UPROPERTY(BlueprintCallable, BlueprintAssignable)
	FOnSquadUpdatedDelegate OnSquadUpdatedDelegate;

	UPROPERTY(BlueprintCallable, BlueprintAssignable)
	FOnCurrentActiveSquadUpdatedDelegate OnCurrentActiveSquadUpdatedDelegate;

private:

	UFUNCTION(Client, reliable)
	void Client_CurrentSquadsUpdated();

	UFUNCTION(Client, reliable)
	void Client_CurrentActiveSquadUpdated(const int32& NewSquadIndex);

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Setup)
	int32 MaxAmountSquads;

	UFUNCTION(BlueprintPure)
	FORCEINLINE TArray<AWOGBaseSquad*> GetCurrentSquads() const { return CurrentSquads; }

	UFUNCTION(BlueprintPure)
	USceneComponent* GetNextAvailableSquadSlot(AWOGBaseSquad* Squad) const;

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void HandleCurrentSquads(AWOGBaseSquad* Squad, bool bAdd);

	UFUNCTION(Server, reliable, BlueprintCallable)
	void Server_SendOrder(AWOGBaseSquad* Squad, const EEnemyOrder& NewOrder, const FTransform& TargetTansform = FTransform(), AActor* TargetActor = nullptr);

	UFUNCTION(BlueprintCallable)
	void SetCurrentlySelectedSquad(AWOGBaseSquad* NewSquad);

	UFUNCTION(BlueprintPure)
	FORCEINLINE AWOGBaseSquad* GetCurrentlySelectedSquad() const { return CurrentlySelectedSquad; }

	UFUNCTION(BlueprintCallable)
	void IncreaseCurrentlySelectedSquad();

	UFUNCTION(BlueprintCallable)
	void DecreaseCurrentlySelectedSquad();

	UPROPERTY(BlueprintCallable, BlueprintAssignable)
	FOnOrderSentDelegate OnOrderSentDelegate;

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void DestroyAllSquads();
};
