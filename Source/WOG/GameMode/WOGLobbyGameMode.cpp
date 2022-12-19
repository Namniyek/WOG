// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGLobbyGameMode.h"
#include "WOG/PlayerController/WOGLobbyPlayerController.h"
#include "WOG/Lobby/WOGLobbyPlayerSpot.h"
#include "WOG/Lobby/WOGLobbyAvatar.h"


AWOGLobbyGameMode::AWOGLobbyGameMode()
{
	PlayersReady = 0;
}

void AWOGLobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	FTimerHandle RegisterDelay;
	FTimerDelegate RegisterDelayDelegate;
	RegisterDelayDelegate.BindUFunction(this, "RegisterPlayer", NewPlayer);
	GetWorld()->GetTimerManager().SetTimer(RegisterDelay, RegisterDelayDelegate, 1.f, false);
}

void AWOGLobbyGameMode::RegisterPlayer(APlayerController* NewPlayer)
{
	AWOGLobbyPlayerController* WOGPlayerController = Cast<AWOGLobbyPlayerController>(NewPlayer);
	if (WOGPlayerController)
	{
		int32 NewIndex = AllLobbyPlayers.Add(WOGPlayerController);

		SetupLobbyCharacter(NewIndex);
	}
}

void AWOGLobbyGameMode::SetupLobbyCharacter(int32 PlayerIndex)
{
	if (AllLobbyPlayers.IsEmpty()) return;

	AWOGLobbyPlayerController* WOGPlayerController = AllLobbyPlayers[PlayerIndex];
	if (!WOGPlayerController) return;
		
	AWOGLobbyPlayerSpot* PlayerSpot;
	int32 TeamIndex;
	if (GetNextPlayerSpot(PlayerSpot, TeamIndex) && PlayerSpot)
	{
		WOGPlayerController->SetLobbyPlayerSpot(PlayerSpot);
		PlayerSpot->SetOwner(WOGPlayerController);
		WOGPlayerController->GetLobbyPlayerSpot()->IsOccupied();
		WOGPlayerController->Server_SetIsAttacker(TeamIndex == 1 ? true : false);

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		AWOGLobbyAvatar* PlayerAvatar = GetWorld()->SpawnActor<AWOGLobbyAvatar>(AvatarBP, PlayerSpot->GetActorLocation(), PlayerSpot->GetActorRotation(), SpawnParams);
		if (PlayerAvatar)
		{
			WOGPlayerController->SetLobbyAvatar(PlayerAvatar);
			WOGPlayerController->GetLobbyAvatar()->SetOwner(WOGPlayerController);
		}
	}
}

void AWOGLobbyGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	AWOGLobbyPlayerController* ExitingPlayer = Cast<AWOGLobbyPlayerController>(Exiting);
	if (!ExitingPlayer) return;
	if (AllLobbyPlayers.Remove(ExitingPlayer))
	{
		ExitingPlayer->GetLobbyPlayerSpot()->IsEmpty();
		ExitingPlayer->GetLobbyAvatar()->Destroy();
		if (ExitingPlayer->GetIsPlayerReady())
		{
			DecreasePlayerReady();
		}
	}
}

bool AWOGLobbyGameMode::GetAttackerPlayerSpot(AWOGLobbyPlayerSpot*& OutPlayerSpot)
{
	if (AttackerLobbySpots.IsEmpty()) return false;
	for (auto LobbySpot : AttackerLobbySpots)
	{
		if (LobbySpot)
		{
			if (LobbySpot->bIsSpotUsed)
			{
				continue;
			}
			else
			{
				OutPlayerSpot = LobbySpot;
				return true;
			}
		}
	}
	return false;
}

bool AWOGLobbyGameMode::GetDefenderPlayerSpot(AWOGLobbyPlayerSpot*& OutPlayerSpot)
{
	if (DefenderLobbySpots.IsEmpty()) return false;
	for (auto LobbySpot : DefenderLobbySpots)
	{
		if (LobbySpot)
		{
			if (LobbySpot->bIsSpotUsed)
			{
				continue;
			}
			else
			{
				OutPlayerSpot = LobbySpot;
				return true;
			}
		}
	}
	return false;
}

bool AWOGLobbyGameMode::GetNextPlayerSpot(AWOGLobbyPlayerSpot* &OutPlayerSpot, int32 &OutTeamIndex)
{
	if (AllLobbySpots.IsEmpty())
	{
		AllLobbySpots.Add(DefenderLobbySpots[0]);
		AllLobbySpots.Add(AttackerLobbySpots[0]);
		AllLobbySpots.Add(DefenderLobbySpots[1]);
		AllLobbySpots.Add(AttackerLobbySpots[1]);
		AllLobbySpots.Add(DefenderLobbySpots[2]);
	}

	for (auto LobbySpot : AllLobbySpots)
	{
		if (LobbySpot)
		{
			if (LobbySpot->bIsSpotUsed)
			{
				continue;
			}
			else
			{
				OutPlayerSpot = LobbySpot;
				OutTeamIndex = LobbySpot->TeamIndex;
				return true;
			}
		}
	}
	return false;
}
