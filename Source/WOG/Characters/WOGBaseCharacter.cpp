// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGBaseCharacter.h"

AWOGBaseCharacter::AWOGBaseCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

}

void AWOGBaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void AWOGBaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWOGBaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

