// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WOGBaseEnemy.h"
#include "InputActionValue.h"
#include "WOGPossessableEnemy.generated.h"

/**
 * 
 */
UCLASS()
class WOG_API AWOGPossessableEnemy : public AWOGBaseEnemy
{
	GENERATED_BODY()
public:
	AWOGPossessableEnemy();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	#pragma region Player Input Variables
	/*
	** MappingContexts
	**/

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup|Input|Base Match")
	class UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup|Input|Base Match")
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup|Input|Base Match")
	UInputAction* PrimaryAttackAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup|Input|Base Match")
	UInputAction* MainAltAttackAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup|Input|Base Match")
	UInputAction* SecondaryAltAttackAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup|Input|Base Match")
	UInputAction* BlockAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup|Input|Base Match")
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup|Input|Base Match")
	UInputAction* UnpossessAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup|Input|Base Match")
	UInputAction* SprintAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup|Input|Base Match")
	UInputAction* RangedAttackAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup|Input|Base Match")
	UInputAction* CloseAttackAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup|Input|Base Match")
	UInputAction* TargetAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup|Input|Base Match")
	UInputAction* CycleTargetAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup|Input|Base Match")
	UInputAction* DodgeAction;
	#pragma endregion

protected:
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void UnpossessMinion_Implementation();

	#pragma region Components variables
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class UTargetSystemComponent* TargetComponent;

	#pragma endregion

	#pragma region Player Input Functions

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** Called for movement input */
	void MoveActionPressed(const FInputActionValue& Value);
	void MoveActionReleased();

	/** Called for looking input */
	void LookActionPressed(const FInputActionValue& Value);

	/** Called for jumping input */
	void JumpActionPressed(const FInputActionValue& Value);

	void PrimaryAttackActionPressed(const FInputActionValue& Value);
	void MainAltAttackActionPressed(const FInputActionValue& Value);
	void SecondaryAltAttackActionPressed(const FInputActionValue& Value);
	void RangedAttackActionPressed(const FInputActionValue& Value);
	void CloseAttackActionPressed(const FInputActionValue& Value);
	void BlockActionPressed(const FInputActionValue& Value);
	void BlockActionReleased(const FInputActionValue& Value);
	
	void UnpossessActionPressed(const FInputActionValue& Value);
	void SprintActionPressed(const FInputActionValue& Value);
	void StopSprinting();

	/**Called for Target input*/
	void TargetActionPressed(const FInputActionValue& Value);
	void CycleTargetActionPressed(const FInputActionValue& Value);

	/**Called for dodge input*/
	void DodgeActionPressed(const FInputActionValue& Value);

	#pragma endregion

	#pragma region TimeOfDay
	virtual void HandleTODChange() override;

	UFUNCTION()
	void KeyTimeHit(int32 CurrentTime);
	#pragma endregion

	#pragma region Handle Elim
	virtual void Elim(bool bPlayerLeftGame) override;
	#pragma endregion

	#pragma region Targeting
	UFUNCTION()
	void TargetLocked(AActor* NewTarget);

	UFUNCTION()
	void TargetUnlocked(AActor* OldTarget);

	UFUNCTION(Server, reliable)
	void Server_SetCurrentTarget(AActor* NewTarget = nullptr);

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Replicated)
	TObjectPtr<AActor> CurrentTarget = nullptr;

	bool bIsTargeting = false;

	virtual void ToggleStrafeMovement(bool bIsStrafe) override;
	#pragma endregion

	#pragma region Interface functions
	bool CanBePossessed_Implementation() const;
	#pragma endregion

public:
	UFUNCTION(Server, reliable, BlueprintCallable)
	virtual void Server_UnpossessMinion();
};
