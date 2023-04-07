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

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	void PossessMinion();
	
#pragma region Player Input Functions

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void PossessActionPressed(const FInputActionValue& Value);

#pragma endregion

};
