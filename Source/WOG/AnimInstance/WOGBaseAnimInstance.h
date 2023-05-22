// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "WOGBaseAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class WOG_API UWOGBaseAnimInstance : public UAnimInstance
{
	GENERATED_BODY()


public:

	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;


protected:

	UPROPERTY(BlueprintReadWrite, Category = "Character|General", meta = (AllowPrivateAccess = "true"))
	class ABasePlayerCharacter* PlayerCharacter;

	UPROPERTY(BlueprintReadWrite, Category = "Character|General", meta = (AllowPrivateAccess = "true"))
	class UCharacterMovementComponent* CharacterMovementComponent;

	UPROPERTY(BlueprintReadOnly, Category = "Character|General", meta = (AllowPrivateAccess = "true"))
	FVector Velocity;

	UPROPERTY(BlueprintReadOnly, Category = "Character|General", meta = (AllowPrivateAccess = "true"))
	FRotator Rotation;

	UPROPERTY(BlueprintReadOnly, Category = "Character|General", meta = (AllowPrivateAccess = "true"))
	float GroundSpeed;

	UPROPERTY(BlueprintReadOnly, Category = "Character|General", meta = (AllowPrivateAccess = "true"))
	float GroundSpeedLastFrame;

	UPROPERTY(BlueprintReadOnly, Category = "Character|General", meta = (AllowPrivateAccess = "true"))
	float LateralSpeed;

	UPROPERTY(BlueprintReadOnly, Category = "Character|General", meta = (AllowPrivateAccess = "true"))
	float CurrentRotationRate;

	UPROPERTY(BlueprintReadOnly, Category = "Character|General", meta = (AllowPrivateAccess = "true"))
	float MovementDirectionValue;

	UPROPERTY(BlueprintReadOnly, Category = "Character|General", meta = (AllowPrivateAccess = "true"))
	bool bShouldMove;

	UPROPERTY(BlueprintReadOnly, Category = "Character|General", meta = (AllowPrivateAccess = "true"))
	bool bIsMoving;

	UPROPERTY(BlueprintReadOnly, Category = "Character|General", meta = (AllowPrivateAccess = "true"))
	bool bIsFalling;

	UPROPERTY(BlueprintReadOnly, Category = "Character|General", meta = (AllowPrivateAccess = "true"))
	bool bIsTargeting;

	UPROPERTY(BlueprintReadWrite, Category = "Character|General", meta = (AllowPrivateAccess = "true"))
	bool bStopFootLeft;

	UPROPERTY(BlueprintReadWrite, Category = "Character|General", meta = (AllowPrivateAccess = "true"))
	bool bIsLandingWhileMoving;

	UPROPERTY(BlueprintReadOnly, Category = "Character|General", meta = (AllowPrivateAccess = "true"))
	FName MovementDirection;

	//whether the character is accelerating or not
	UPROPERTY(BlueprintReadOnly, Category = "Character|General", meta = (AllowPrivateAccess = "true"))
	bool bIsAccelerating;

	UPROPERTY(BlueprintReadOnly, Category = "Character|General", meta = (AllowPrivateAccess = "true"))
	float SpeedRequiredForLeap;

	void UpdateMovementDirection();

public:
	UFUNCTION(BlueprintPure)
	FORCEINLINE FName GetMovementDirection() const { return MovementDirection; }

	
};
