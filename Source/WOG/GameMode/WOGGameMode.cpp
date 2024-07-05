// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGGameMode.h"
#include "WOG.h"
#include "WOG/GameInstance/WOGGameInstance.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "WOG/Characters/WOGBaseCharacter.h"
#include "WOG/PlayerCharacter/WOGAttacker.h"
#include "WOG/PlayerCharacter/WOGDefender.h"
#include "WOG/PlayerController/WOGPlayerController.h"
#include "WOG/PlayerState/WOGPlayerState.h"
#include "WOG/UI/WOGMatchHUD.h"
#include "WOG/GameState/WOGGameState.h"
#include "Engine/Engine.h"
#include "OnlineSubsystemUtils.h"


void AWOGGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);
	bHandleDropIn = bDebugMode;
	if (!bHandleDropIn)
	{
		HandleStartingPlayer(NewPlayer);
	}
	else
	{
		HandleDropIn(NewPlayer);
	}
}

void AWOGGameMode::HandleStartingPlayer(APlayerController* NewPlayer)
{
	GameInstance = GetGameInstance<UWOGGameInstance>();
	if (!GameInstance)
	{
		return;
	}

	TArray <FString> PlayerNameArray;
	GameInstance->PlayersMap.GenerateValueArray(PlayerNameArray);
	FString DesiredPlayerName = NewPlayer->PlayerState->GetPlayerName();

	if (PlayerNameArray.IsEmpty())
	{
		return;
	}

	for (int32 i = 0; i < PlayerNameArray.Num(); i++)
	{
		FString PlayerName = PlayerNameArray[i];
		if (PlayerName != DesiredPlayerName)
		{
			continue;
		}

		if (i <= LastDefenderIndex)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = NewPlayer;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
			ABasePlayerCharacter* Defender = 
				GetWorld()->SpawnActor<AWOGDefender>(DefenderCharacter, GetPlayerStart(FString::FromInt(i)), SpawnParams);
			if (Defender)
			{
				NewPlayer->Possess(Cast<APawn>(Defender));
				Defender->SetOwner(NewPlayer);
			}
		}
		else
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = NewPlayer;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
			ABasePlayerCharacter* Attacker = 
				GetWorld()->SpawnActor<AWOGAttacker>(AttackerCharacter, GetPlayerStart(FString::FromInt(i)), SpawnParams);
			if (Attacker)
			{
				NewPlayer->Possess(Cast<APawn>(Attacker));
				Attacker->SetOwner(NewPlayer);
			}
		}
	}
}

void AWOGGameMode::HandleDropIn(APlayerController* NewPlayer)
{
	CreateRandomCharacter(NewPlayer);
	HandleStartingPlayer(NewPlayer);
}

void AWOGGameMode::CreateRandomCharacter(APlayerController* NewPlayer)
{
	GameInstance = GetGameInstance<UWOGGameInstance>();
	if (!GameInstance)
	{
		return;
	}

	TArray <FString> PlayerNameArray;
	GameInstance->PlayersMap.GenerateValueArray(PlayerNameArray);
	FString DesiredPlayerName = NewPlayer->PlayerState->GetPlayerName();

	if (PlayerNameArray.IsEmpty())
	{
		return;
	}

	for (int32 i = 0; i < PlayerNameArray.Num(); i++)
	{
		FString PlayerName = PlayerNameArray[i];
		if (PlayerName == FString("empty"))
		{
			GameInstance->PlayersMap.Add(i, DesiredPlayerName);
			UPlayerProfileSaveGame* SaveGameObject = Cast<UPlayerProfileSaveGame>(UGameplayStatics::CreateSaveGameObject(UPlayerProfileSaveGame::StaticClass()));
			if (SaveGameObject)
			{
				FString CharIndex = FString::FromInt(DebugCharacterIndex);
				SaveGameObject->PlayerProfile.CharacterIndex = FName(*CharIndex);
				
				SaveGameObject->PlayerProfile.bIsAttacker = (i>LastDefenderIndex);
				SaveGameObject->PlayerProfile.bIsMale = bIsDebugCharacterMale;
				SaveGameObject->PlayerProfile.BodyPaintColor = "0";
				SaveGameObject->PlayerProfile.HairColor = "0";
				SaveGameObject->PlayerProfile.PrimaryColor = "0";
				SaveGameObject->PlayerProfile.Rune = "0";
				SaveGameObject->PlayerProfile.SkinColor = "0";
				SaveGameObject->PlayerProfile.UserIndex = i;
				SaveGameObject->PlayerProfile.PlayerName = DesiredPlayerName;

				UGameplayStatics::SaveGameToSlot(SaveGameObject, SaveGameObject->PlayerProfile.PlayerName, SaveGameObject->PlayerProfile.UserIndex);
				break;
			}
		}
	}
	return;
}

FTransform AWOGGameMode::GetPlayerStart(FString StartIndex)
{
	FName StartIndexName = (*StartIndex);
	TArray<AActor*> PlayerStartArray;
	UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStartArray);

	if (PlayerStartArray.IsEmpty()) return FTransform();

	for (auto PlayerStart : PlayerStartArray)
	{
		APlayerStart* Start = Cast<APlayerStart>(PlayerStart);
		if (Start)
		{
			if (StartIndexName == Start->PlayerStartTag)
			{
				return Start->GetActorTransform();
			}
		}
	}
	return FTransform();
}

void AWOGGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	if (GetMatchState() != MatchState::InProgress) return;

	GameInstance = GetGameInstance<UWOGGameInstance>();
	if (!GameInstance) return;

	AWOGPlayerController* PlayerController = Cast<AWOGPlayerController>(Exiting);
	if (PlayerController)
	{
		GameInstance->PlayersMap.Add(PlayerController->UserIndex, FString("empty"));
	}
	bHandleDropIn = true;
}

void AWOGGameMode::RestartMatch()
{
	RestartGame();
}

void AWOGGameMode::PlayerEliminated(AWOGBaseCharacter* ElimmedCharacter, AWOGPlayerController* VictimController, AWOGPlayerController* AttackerController)
{
	if (!VictimController || !AttackerController) return;
	AWOGPlayerState* AttackerPlayerState = AttackerController ? AttackerController->GetPlayerState<AWOGPlayerState>() : nullptr;
	AWOGPlayerState* VictimPlayerState = VictimController ? VictimController->GetPlayerState<AWOGPlayerState>() : nullptr;

	if (!AttackerPlayerState || !VictimPlayerState) return;
	AttackerPlayerState->IncreaseTotalElimms();
	VictimPlayerState->IncreaseTimesElimmed();

	AWOGGameState* WOGGameState = GetGameState<AWOGGameState>();
	if (!WOGGameState) return;
	WOGGameState->Server_SetEndgamePlayerStats();

	if (ElimmedCharacter)
	{
		ElimmedCharacter->Elim(false);
	}
}

void AWOGGameMode::PreLogout(APlayerController* InPlayerController)
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

void AWOGGameMode::RequestRespawn(ABasePlayerCharacter* ElimmedCharacter, APlayerController* ElimmedController)
{
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Destroy();
	}

	if (ElimmedController)
	{
		HandleStartingPlayer(ElimmedController);
	}
}

