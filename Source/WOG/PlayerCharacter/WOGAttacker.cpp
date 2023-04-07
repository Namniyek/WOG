// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGAttacker.h"
#include "TargetingHelperComponent.h"
#include "LockOnTargetComponent.h"
#include "WOG/ActorComponents/WOGAttributesComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "WOG/PlayerController/WOGPlayerController.h"

void AWOGAttacker::BeginPlay()
{
	Super::BeginPlay();

	if (Attributes)
	{
		Attributes->Server_PassiveAttributeUpdate(EAttributeType::AT_Mana, 2.f);
	}
}

void AWOGAttacker::PossessMinion()
{
	OwnerPC = OwnerPC == nullptr ? Cast<AWOGPlayerController>(GetController()) : OwnerPC;
	if (!OwnerPC)
	{
		UE_LOG(LogTemp, Error, TEXT("invalid OwnerPC"));
		return;
	}

	if (!CurrentTarget)
	{
		UE_LOG(LogTemp, Error, TEXT("invalid CurrentTarget"));
		return;
	}

	OwnerPC->PossessMinion(CurrentTarget);
	LockOnTarget->EnableTargeting();
}

void AWOGAttacker::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		//Possess
		EnhancedInputComponent->BindAction(PossessAction, ETriggerEvent::Completed, this, &ThisClass::PossessActionPressed);
	}
}

void AWOGAttacker::PossessActionPressed(const FInputActionValue& Value)
{
	if (CharacterState == ECharacterState::ECS_Elimmed) return;
	PossessMinion();
}
