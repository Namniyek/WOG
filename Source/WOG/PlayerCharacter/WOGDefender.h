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

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable)
	void DestroyComponent(UActorComponent* ComponentToDestroy);

	#pragma region Player Input Variables
	/*
	** MappingContexts
	**/


	/*
	** Input actions
	**/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup|Input|Spawn Mode", meta = (AllowPrivateAccess = "true"))
	UInputAction* AdjustSpawnHeightAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup|Input|Base Match", meta = (AllowPrivateAccess = "true"))
	UInputAction* InteractAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup|Input|Base Match", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> Ability3HoldAction;
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

	virtual void AbilitiesButtonPressed(const FInputActionValue& Value) override;

	virtual void Ability3HoldButtonStarted(const FInputActionValue& Value) override;
	virtual void Ability3HoldButtonTriggered(const FInputActionValue& Value) override;

	#pragma endregion

	#pragma region Interface functions
	bool IsCurrentMeleeSquadSlotAvailable_Implementation() const;
	bool IsCurrentRangedSquadSlotAvailable_Implementation() const;
	void FreeCurrentRangedSquadSlot_Implementation();
	void FreeCurrentMeleeSquadSlot_Implementation();
	void SetCurrentRangedSquadSlot_Implementation(AWOGBaseSquad* NewSquad);
	void SetCurrentMeleeSquadSlot_Implementation(AWOGBaseSquad* NewSquad);
	AWOGBaseSquad* GetCurrentRangedSquadSlot_Implementation() const;
	AWOGBaseSquad* GetCurrentMeleeSquadSlot_Implementation() const;
	#pragma endregion

	UPROPERTY(Replicated, VisibleAnywhere)
	TObjectPtr<AWOGBaseSquad> CurrentMeleeSquad;

	UPROPERTY(Replicated, VisibleAnywhere)
	TObjectPtr<AWOGBaseSquad> CurrentRangedSquad;

public:
	UFUNCTION(BlueprintCallable)
	FORCEINLINE UWOGBuildComponent* GetBuildComponent() { return BuildComponent; }

	UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable)
	void SetCurrentRangedSquad(AWOGBaseSquad* NewSquad);

	UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable)
	void SetCurrentMeleeSquad(AWOGBaseSquad* NewSquad);
	
};
