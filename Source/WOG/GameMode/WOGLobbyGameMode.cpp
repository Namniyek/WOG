// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGLobbyGameMode.h"
#include "WOG.h"
#include "WOG/PlayerController/WOGLobbyPlayerController.h"
#include "WOG/Lobby/WOGLobbyPlayerSpot.h"
#include "WOG/Lobby/WOGLobbyAvatar.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineSessionInterface.h"


AWOGLobbyGameMode::AWOGLobbyGameMode()
{
	PlayersReady = 0;
	bAllExistingPlayersRegistered = false;
}

void AWOGLobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	if (bAllExistingPlayersRegistered)
	{
		check(IsValid(NewPlayer));

		// This code handles logins for both the local player (listen server) and remote players (net connection).
		FUniqueNetIdRepl UniqueNetIdRepl;
		if (NewPlayer->IsLocalPlayerController())
		{
			ULocalPlayer *LocalPlayer = NewPlayer->GetLocalPlayer();
			if (IsValid(LocalPlayer))
			{
				UniqueNetIdRepl = LocalPlayer->GetPreferredUniqueNetId();
			}
			else
			{
				UNetConnection *RemoteNetConnection = Cast<UNetConnection>(NewPlayer->Player);
				check(IsValid(RemoteNetConnection));
				UniqueNetIdRepl = RemoteNetConnection->PlayerId;
			}
		}
		else
		{
			UNetConnection *RemoteNetConnection = Cast<UNetConnection>(NewPlayer->Player);
			check(IsValid(RemoteNetConnection));
			UniqueNetIdRepl = RemoteNetConnection->PlayerId;
		}

		// Get the unique player ID.
		TSharedPtr<const FUniqueNetId> UniqueNetId = UniqueNetIdRepl.GetUniqueNetId();
		check(UniqueNetId != nullptr);

		// Get the online session interface.
		IOnlineSubsystem *Subsystem = Online::GetSubsystem(NewPlayer->GetWorld());
		IOnlineSessionPtr Session = Subsystem->GetSessionInterface();

		// Register the player with the "MyLocalSessionName" session; this name should match the name you provided in CreateSession.
		if (!Session->RegisterPlayer(WOG_SESSION_NAME, *UniqueNetId, false))
		{
			// The player could not be registered; typically you will want to kick the player from the server in this situation.
		}
	}
	
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

void AWOGLobbyGameMode::PreLogout(APlayerController* InPlayerController)
{
	check(IsValid(InPlayerController));

	// This code handles logins for both the local player (listen server) and remote players (net connection).
	FUniqueNetIdRepl UniqueNetIdRepl;
	if (InPlayerController->IsLocalPlayerController())
	{
		ULocalPlayer *LocalPlayer = InPlayerController->GetLocalPlayer();
		if (IsValid(LocalPlayer))
		{
			UniqueNetIdRepl = LocalPlayer->GetPreferredUniqueNetId();
		}
		else
		{
			UNetConnection *RemoteNetConnection = Cast<UNetConnection>(InPlayerController->Player);
			check(IsValid(RemoteNetConnection));
			UniqueNetIdRepl = RemoteNetConnection->PlayerId;
		}
	}
	else
	{
		UNetConnection *RemoteNetConnection = Cast<UNetConnection>(InPlayerController->Player);
		check(IsValid(RemoteNetConnection));
		UniqueNetIdRepl = RemoteNetConnection->PlayerId;
	}

	// Get the unique player ID.
	TSharedPtr<const FUniqueNetId> UniqueNetId = UniqueNetIdRepl.GetUniqueNetId();
	check(UniqueNetId != nullptr);

	// Get the online session interface.
	IOnlineSubsystem *Subsystem = Online::GetSubsystem(InPlayerController->GetWorld());
	IOnlineSessionPtr Session = Subsystem->GetSessionInterface();

	// Unregister the player with the "MyLocalSessionName" session; this name should match the name you provided in CreateSession.
	if (!Session->UnregisterPlayer(WOG_SESSION_NAME, *UniqueNetId))
	{
		// The player could not be unregistered.
		GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Red, FString("Failed to call Unregister player"));
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

void AWOGLobbyGameMode::RegisterExistingPlayers()
{
	for (auto It = this->GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PlayerController = It->Get();
        
		FUniqueNetIdRepl UniqueNetIdRepl;
		if (PlayerController->IsLocalPlayerController())
		{
			ULocalPlayer *LocalPlayer = PlayerController->GetLocalPlayer();
			if (IsValid(LocalPlayer))
			{
				UniqueNetIdRepl = LocalPlayer->GetPreferredUniqueNetId();
			}
			else
			{
				UNetConnection *RemoteNetConnection = Cast<UNetConnection>(PlayerController->Player);
				check(IsValid(RemoteNetConnection));
				UniqueNetIdRepl = RemoteNetConnection->PlayerId;
			}
		}
		else
		{
			UNetConnection *RemoteNetConnection = Cast<UNetConnection>(PlayerController->Player);
			check(IsValid(RemoteNetConnection));
			UniqueNetIdRepl = RemoteNetConnection->PlayerId;
		}

		// Get the unique player ID.
		TSharedPtr<const FUniqueNetId> UniqueNetId = UniqueNetIdRepl.GetUniqueNetId();
		check(UniqueNetId != nullptr);

		// Get the online session interface.
		IOnlineSubsystem *Subsystem = Online::GetSubsystem(PlayerController->GetWorld());
		IOnlineSessionPtr Session = Subsystem->GetSessionInterface();

		// Register the player with the name matching the name provided in CreateSession.
		if (!Session->RegisterPlayer(WOG_SESSION_NAME, *UniqueNetId, false))
		{
			// The player could not be registered; typically you will want to kick the player from the server in this situation.
			GEngine->AddOnScreenDebugMessage(0, 6.f, FColor::Red, FString("Failed to register player"));
		}
	}

	bAllExistingPlayersRegistered = true;
}
