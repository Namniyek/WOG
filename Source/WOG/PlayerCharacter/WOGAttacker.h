// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BasePlayerCharacter.h"
#include "WOGAttacker.generated.h"

/**
 * 
 */
UCLASS()
class WOG_API AWOGAttacker : public ABasePlayerCharacter
{
	GENERATED_BODY()


public:
	AWOGAttacker();
	friend class UWOGSpawnComponent;

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


public:
	UFUNCTION(BlueprintCallable)
	UWOGSpawnComponent* GetSpawnComponent() { return SpawnComponent; }
};
