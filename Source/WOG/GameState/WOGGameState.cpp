// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGGameState.h"

void AWOGGameState::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();
	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, FString("StartedGame"));
}
