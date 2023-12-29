// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BasePlayerCharacter.h"
#include "WOGAttacker.generated.h"

/**
 * 
 */

class AWOGRaven;
class AWOGMinerGiant;
class AWOGPossessableEnemy;
class UWOGEnemyOrderComponent;

UCLASS()
class WOG_API AWOGAttacker : public ABasePlayerCharacter
{
	GENERATED_BODY()


public:
	AWOGAttacker();
	friend class UWOGSpawnComponent;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<USceneComponent> SquadSlot_0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<USceneComponent> SquadSlot_1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<USceneComponent> SquadSlot_2;
	#pragma region Player Input Variables
	/*
	** MappingContexts
	**/

	/*
	** Input actions
	**/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup|Input|Base Match", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> PossessAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup|Input|Base Match", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> Ability2HoldAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup|Input|Base Match", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> Ability3HoldAction;

	#pragma endregion

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UWOGEnemyOrderComponent> EnemyOrderComponent;

	UFUNCTION(BlueprintAuthorityOnly)
	void SetAllocatedRaven();

	UFUNCTION(BlueprintAuthorityOnly)
	void SetAllocatedMiner();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UWOGSpawnComponent> SpawnComponent;

	UFUNCTION(BlueprintCallable)
	void PossessMinion();
	
	#pragma region Player Input Functions

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void PossessActionPressed(const FInputActionValue& Value);
	/**Called for spawn input*/
	void RotateSpawnActionPressed(const FInputActionValue& Value);
	void SpawnActionPressed(const FInputActionValue& Value);

	virtual void AbilitiesButtonPressed(const FInputActionValue& Value) override;

	virtual void Ability2HoldButtonStarted(const FInputActionValue& Value) override;
	virtual void Ability2HoldButtonTriggered(const FInputActionValue& Value) override;
	virtual void Ability3HoldButtonStarted(const FInputActionValue& Value) override;
	virtual void Ability3HoldButtonTriggered(const FInputActionValue& Value) override;

	#pragma endregion

	UPROPERTY(Replicated, VisibleAnywhere)
	TObjectPtr<AWOGRaven> Raven;

	UPROPERTY(Replicated, VisibleAnywhere)
	TObjectPtr<AWOGMinerGiant> Miner;

	UFUNCTION(BlueprintCallable)
	void PossessRaven();

	UFUNCTION(BlueprintCallable)
	void PossessMiner();

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
	bool bCanPossessMinion;

	virtual void OnHealthAttributeChanged(const FOnAttributeChangeData& Data) override;

private:
	UPROPERTY(Replicated, VisibleAnywhere)
	TObjectPtr<AActor> CurrentExternalMinion;

public:
	UFUNCTION(BlueprintCallable)
	UWOGSpawnComponent* GetSpawnComponent() { return SpawnComponent; }

	void SetCurrentExternalMinion(AActor* NewMinion);

	UFUNCTION(BlueprintPure)
	FORCEINLINE UWOGEnemyOrderComponent* GetEnemyOrderComponent() const { return EnemyOrderComponent; }
};
