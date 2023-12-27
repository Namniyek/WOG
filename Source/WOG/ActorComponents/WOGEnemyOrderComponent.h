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

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void SendOrder(AWOGBaseSquad* Squad, const EEnemyOrder& NewOrder, const FTransform& TargetTansform = FTransform(), AActor* TargetActor = nullptr);

public:
	UFUNCTION(BlueprintPure)
	FORCEINLINE TArray<AWOGBaseSquad*> GetCurrentSquads() const { return CurrentSquads; }

	UFUNCTION(BlueprintPure)
	USceneComponent* GetNextAvailableSquadSlot() const;

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void HandleCurrentSquads(AWOGBaseSquad* Squad, bool bAdd);

	UFUNCTION(Server, reliable, BlueprintCallable)
	void Server_SendOrder(AWOGBaseSquad* Squad, const EEnemyOrder& NewOrder, const FTransform& TargetTansform = FTransform(), AActor* TargetActor = nullptr);
};
