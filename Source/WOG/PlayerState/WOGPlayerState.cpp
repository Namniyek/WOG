// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGPlayerState.h"
#include "Net/UnrealNetwork.h"

AWOGPlayerState::AWOGPlayerState()
{
	PlayerStats.TimesElimmed = 0;
	PlayerStats.TotalElimms = 0;
	PlayerStats.MostElimmedPlayer = FString("DEFAULT");
	PlayerStats.PlayerWithMostElimms = FString("DEFAULT");

	bIsAttacker = false;

	APlayerState::SetPlayerName(FString("Default"));
}

void AWOGPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWOGPlayerState, PlayerStats);
	DOREPLIFETIME(AWOGPlayerState, bIsAttacker)
	DOREPLIFETIME(AWOGPlayerState, PlayerCharacter);
}

void AWOGPlayerState::IncreaseTimesElimmed()
{
	PlayerStats.TimesElimmed++;
}

void AWOGPlayerState::IncreaseTotalElims()
{
	PlayerStats.TotalElimms++;
}

void AWOGPlayerState::SetMostElimmedPlayer(FString Player)
{
	PlayerStats.MostElimmedPlayer = Player;
}

void AWOGPlayerState::SetPlayerWithMostElims(FString Player)
{
	PlayerStats.PlayerWithMostElimms = Player;
}

void AWOGPlayerState::SetIsAttacker(const bool bNewAttacker)
{
	if(bNewAttacker != bIsAttacker) bIsAttacker = bNewAttacker;
}

void AWOGPlayerState::SetPlayerCharacter(ABasePlayerCharacter* NewPlayerChar)
{
	if(PlayerCharacter != NewPlayerChar) PlayerCharacter = NewPlayerChar;
}
