// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGBaseAnimInstance.h"
#include "WOG/PlayerCharacter/BasePlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

void UWOGBaseAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	PlayerCharacter = Cast<ABasePlayerCharacter>(GetOwningActor());
	if (PlayerCharacter)
	{
		CharacterMovementComponent = PlayerCharacter->GetCharacterMovement();
		GEngine->AddOnScreenDebugMessage(1, -1, FColor::Red, FString::FromInt(PlayerCharacter == nullptr));
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(1, -1, FColor::Red, FString::FromInt(PlayerCharacter == nullptr));
	}
	GEngine->AddOnScreenDebugMessage(1, -1, FColor::Red, FString("InitializeAnimation"));
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
	
	//Get the velocity vector and the ground speed of the character 
	Velocity = PlayerCharacter->GetVelocity();
	GroundSpeed = Velocity.Size2D();

	//Check if the character should move
	bShouldMove = (
		CharacterMovementComponent->GetCurrentAcceleration() == FVector()
		&& GroundSpeed > 3.f);

	//Check if the character is falling
	bIsFalling = CharacterMovementComponent->IsFalling();

	//Get the character rotation
	Rotation = PlayerCharacter->GetActorRotation();

	//Get the bool bIsTargeting
	bIsTargeting = PlayerCharacter->GetIsTargeting();

	#pragma endregion


}
