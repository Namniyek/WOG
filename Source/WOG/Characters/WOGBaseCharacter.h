// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "WOGBaseCharacter.generated.h"

UCLASS()
class WOG_API AWOGBaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AWOGBaseCharacter();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
