// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGPlayerState.h"
#include "Net/UnrealNetwork.h"

AWOGPlayerState::AWOGPlayerState()
{
	PlayerStats.TimesElimmed = 0;
	PlayerStats.TotalElimms = 0;
	PlayerStats.MostElimmedPlayer = FString("DEFAULT");
	PlayerStats.PlayerWithMostElimms = FString("DEFAULT");

}

void AWOGPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWOGPlayerState, PlayerStats);
}

void AWOGPlayerState::IncreaseTimesElimmed()
{
	PlayerStats.TimesElimmed++;
}

void AWOGPlayerState::IncreaseTotalElimms()
{
	PlayerStats.TotalElimms++;
}

void AWOGPlayerState::SetMostElimmedPlayer(FString Player)
{
	PlayerStats.MostElimmedPlayer = Player;
}

void AWOGPlayerState::SetPlayerWithMostElimms(FString Player)
{
	PlayerStats.PlayerWithMostElimms = Player;
}
