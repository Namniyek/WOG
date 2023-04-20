// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BasePlayerCharacter.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "WOGDefender.generated.h"

/**
 * 
 */
UCLASS()
class WOG_API AWOGDefender : public ABasePlayerCharacter
{
	GENERATED_BODY()

public:
	AWOGDefender();
	friend class UWOGBuildComponent;

	UFUNCTION(BlueprintCallable)
	void DestroyComponent(UActorComponent* ComponentToDestroy);

	#pragma region Player Input Variables
	/*
	** MappingContexts
	**/


	/*
	** Input actions
	**/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Spawn Mode", meta = (AllowPrivateAccess = "true"))
	UInputAction* AdjustSpawnHeightAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Base Match", meta = (AllowPrivateAccess = "true"))
	UInputAction* InteractAction;

	#pragma endregion

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UWOGBuildComponent> BuildComponent;

	#pragma region Player Input Functions

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/**Called for interact input*/
	void InteractActionPressed(const FInputActionValue& Value);

	/**Called for spawn input*/
	void AdjustSpawnHeightActionPressed(const FInputActionValue& Value);
	void RotateSpawnActionPressed(const FInputActionValue& Value);
	void SpawnActionPressed(const FInputActionValue& Value);

	#pragma endregion

public:
	UFUNCTION(BlueprintCallable)
	FORCEINLINE UWOGBuildComponent* GetBuildComponent() { return BuildComponent; }
	
};
