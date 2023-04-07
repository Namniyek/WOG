// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGGameMode.h"
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

		if (i <= 0)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = NewPlayer;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
			ABasePlayerCharacter* Defender = 
				GetWorld()->SpawnActor<AWOGDefender>(DefenderCharacter, GetPlayerStart(FString::FromInt(i)), SpawnParams);
			if (Defender)
			{
				NewPlayer->Possess(Cast<APawn>(Defender));
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
				SaveGameObject->PlayerProfile.bIsAttacker = (i>0);
				SaveGameObject->PlayerProfile.bIsMale = true; //FMath::RandBool();
				SaveGameObject->PlayerProfile.BodyPaintColor = "0";
				SaveGameObject->PlayerProfile.CharacterIndex = "1";
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

