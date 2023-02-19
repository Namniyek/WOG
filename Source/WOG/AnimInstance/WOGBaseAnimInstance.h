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
	bool bShouldMove;

	UPROPERTY(BlueprintReadOnly, Category = "Character|General", meta = (AllowPrivateAccess = "true"))
	bool bIsFalling;

	UPROPERTY(BlueprintReadOnly, Category = "Character|General", meta = (AllowPrivateAccess = "true"))
	bool bIsTargeting;
	
};
