// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGBaseAnimInstance.h"
#include "WOG/PlayerCharacter/BasePlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "KismetAnimationLibrary.h"
#include "Components/CapsuleComponent.h"

void UWOGBaseAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	PlayerCharacter = Cast<ABasePlayerCharacter>(GetOwningActor());
	if (PlayerCharacter)
	{
		CharacterMovementComponent = PlayerCharacter->GetCharacterMovement();
		SpeedRequiredForLeap = PlayerCharacter->GetSpeedRequiredForLeap();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("No valid PlayerCharacter in anim instance"));
	}
}

void UWOGBaseAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	#pragma region Base variables
	//Setup the base variables incase they are not valid
	if (PlayerCharacter == nullptr)
	{
		PlayerCharacter = Cast<ABasePlayerCharacter>(GetOwningActor());
		if (PlayerCharacter)
		{
			CharacterMovementComponent = PlayerCharacter->GetCharacterMovement();
		}
	}

	if (PlayerCharacter == nullptr || CharacterMovementComponent == nullptr) return;
	
	//is accelerating
	if (PlayerCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0)
	{
		bIsAccelerating = true;
	}
	else
	{
		bIsAccelerating = false;
	}

	//Get the ground speed just before stopping
	if (!bIsAccelerating)
	{
		GroundSpeedLastFrame = GroundSpeed;
	}

	//Get the ground and lateral speed of the character 
	Velocity = PlayerCharacter->GetVelocity();
	GroundSpeed = Velocity.Size2D();
	LateralSpeed = UKismetMathLibrary::VSizeXY(Velocity);

	bIsMoving = Velocity != FVector(0.f);

	//Check if the character should move
	bShouldMove = (
		CharacterMovementComponent->GetCurrentAcceleration() == FVector()
		&& GroundSpeed > 3.f);

	//Check if the character is falling
	bIsFalling = CharacterMovementComponent->IsFalling();

	//Get the character rotation
	Rotation = PlayerCharacter->GetActorRotation();

	//Get the current rotation rate
	FVector CapsuleAngularVelocity = PlayerCharacter->GetCapsuleComponent()->GetPhysicsAngularVelocityInDegrees();
	CurrentRotationRate = UKismetMathLibrary::MapRangeClamped(CapsuleAngularVelocity.Z, -360, 360, -1, 1);

	//Get the bool bIsTargeting
	bIsTargeting = PlayerCharacter->GetIsTargeting();

	UpdateMovementDirection();

	#pragma endregion

}

void UWOGBaseAnimInstance::UpdateMovementDirection()
{
	MovementDirectionValue = UKismetAnimationLibrary::CalculateDirection(Velocity, Rotation);
	if (UKismetMathLibrary::InRange_FloatFloat(MovementDirectionValue, -15.f, 0.f) || UKismetMathLibrary::InRange_FloatFloat(MovementDirectionValue, 0.f, 15.f))
	{
		MovementDirection = FName("Front");
	}
	else if (UKismetMathLibrary::InRange_FloatFloat(MovementDirectionValue, -100.f, -16.f))
	{
		MovementDirection = FName("Left");
	}
	else if (UKismetMathLibrary::InRange_FloatFloat(MovementDirectionValue, 16.f, 100.f))
	{
		MovementDirection = FName("Right");
	}
	else if (UKismetMathLibrary::InRange_FloatFloat(MovementDirectionValue, -180.f, -101.f) || UKismetMathLibrary::InRange_FloatFloat(MovementDirectionValue, 101, 180.f))
	{
		MovementDirection = FName("Back");
	}
}
