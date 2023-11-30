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
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup|Input|Mapping Contexts")
	TObjectPtr<class UInputMappingContext> MinionMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup|Input|Base Match")
	class UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup|Input|Base Match")
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup|Input|Base Match")
	UInputAction* PrimaryLightAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup|Input|Base Match")
	UInputAction* PrimaryHeavyAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup|Input|Base Match")
	UInputAction* SecondaryAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup|Input|Base Match")
	UInputAction* JumpAction;
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

	#pragma endregion

	#pragma region Player Input Functions

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** Called for movement input */
	void MoveActionPressed(const FInputActionValue& Value);

	/** Called for looking input */
	void LookActionPressed(const FInputActionValue& Value);

	/** Called for jumping input */
	void JumpActionPressed(const FInputActionValue& Value);

	UFUNCTION()
	void PrimaryLightButtonPressed(const FInputActionValue& Value);

	UFUNCTION()
	void SecondaryLightButtonPressed(const FInputActionValue& Value);

	#pragma endregion

	#pragma region TimeOfDay
	virtual void HandleTODChange() override;

	UFUNCTION()
	void KeyTimeHit(int32 CurrentTime);
	#pragma endregion

	#pragma region Handle Elim
	virtual void Elim(bool bPlayerLeftGame) override;
	#pragma endregion

public:
	UFUNCTION(Server, reliable, BlueprintCallable)
	virtual void Server_UnpossessMinion();
};
