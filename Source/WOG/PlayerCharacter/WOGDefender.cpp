// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGDefender.h"
#include "TargetingHelperComponent.h"
#include "LockOnTargetComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "WOG/ActorComponents/WOGBuildComponent.h"

AWOGDefender::AWOGDefender()
{
	BuildComponent = CreateDefaultSubobject<UWOGBuildComponent>(TEXT("BuildingComponent"));
	BuildComponent->SetIsReplicated(true);
}

void AWOGDefender::DestroyComponent(UActorComponent* ComponentToDestroy)
{
	if (ComponentToDestroy)
	{
		ComponentToDestroy->DestroyComponent();
	}
}

void AWOGDefender::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		//Interact
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Triggered, this, &ThisClass::InteractActionPressed);

		//Spawn 
		EnhancedInputComponent->BindAction(SpawnAction, ETriggerEvent::Completed, this, &ThisClass::SpawnActionPressed);
		EnhancedInputComponent->BindAction(RotateSpawnAction, ETriggerEvent::Triggered, this, &ThisClass::RotateSpawnActionPressed);
		EnhancedInputComponent->BindAction(AdjustSpawnHeightAction, ETriggerEvent::Triggered, this, &ThisClass::AdjustSpawnHeightActionPressed);
	}
}

void AWOGDefender::InteractActionPressed(const FInputActionValue& Value)
{
	//TO-DO interact functionality
	UE_LOG(LogTemp, Warning, TEXT("Interact button pressed from C++"));
}

void AWOGDefender::AdjustSpawnHeightActionPressed(const FInputActionValue& Value)
{
	if (CharacterState == ECharacterState::ECS_Elimmed) return;
	if (!BuildComponent) return;
	float Direction = Value.Get<float>();

	if (Direction > 0)
	{
		//Raise Build
		BuildComponent->HandleBuildHeight(true);
	}
	else
	{
		//Lower right
		BuildComponent->HandleBuildHeight(false);
	}
}

void AWOGDefender::RotateSpawnActionPressed(const FInputActionValue& Value)
{
	if (CharacterState == ECharacterState::ECS_Elimmed) return;
	if (!BuildComponent) return;
	float Direction = Value.Get<float>();

	if (Direction < 0)
	{
		//Rotate left
		BuildComponent->HandleBuildRotation(true);
	}
	else
	{
		//Rotate right
		BuildComponent->HandleBuildRotation(false);
	}
}

void AWOGDefender::SpawnActionPressed(const FInputActionValue& Value)
{
	if (CharacterState == ECharacterState::ECS_Elimmed) return;
	if (!BuildComponent) return;
	BuildComponent->PlaceBuildable();
}
