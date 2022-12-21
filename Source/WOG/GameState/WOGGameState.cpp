// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGGameState.h"
#include "Engine/NetConnection.h"

void AWOGGameState::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();
	GEngine->AddOnScreenDebugMessage(1, 2.f, FColor::Red, FString("StartedGame"));
}

int32 AWOGGameState::UpdateCurrentTime()
{
	return GetServerWorldTimeSeconds()*2;
}


