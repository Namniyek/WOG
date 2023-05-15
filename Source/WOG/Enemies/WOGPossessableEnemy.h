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
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Mapping Contexts", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputMappingContext> MinionMappingContext;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* PrimaryLightAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* PrimaryHeavyAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SecondaryAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	#pragma endregion

protected:
	virtual void BeginPlay() override;

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

	#pragma endregion

	virtual void PossessedBy(AController* NewController) override;

	UFUNCTION(Server, reliable, BlueprintCallable)
	virtual void Server_UnpossessMinion();
	virtual void Elim(bool bPlayerLeftGame) override;

public:
};
