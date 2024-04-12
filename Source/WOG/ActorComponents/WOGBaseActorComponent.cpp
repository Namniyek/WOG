// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGBaseActorComponent.h"
#include "Net/UnrealNetwork.h"

UWOGBaseActorComponent::UWOGBaseActorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	OwnerPC = nullptr;
}

void UWOGBaseActorComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UWOGBaseActorComponent, OwnerCharacter);
}

void UWOGBaseActorComponent::BeginPlay()
{
	Super::BeginPlay();
	
}


