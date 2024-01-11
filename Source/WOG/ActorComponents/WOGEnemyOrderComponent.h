// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActorComponents/WOGBaseActorComponent.h"
#include "Data/WOGDataTypes.h"
#include "WOGEnemyOrderComponent.generated.h"

/**
 * 
 */
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

private:

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
};
