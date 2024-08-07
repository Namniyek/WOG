// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGLobbyPlayerController.h"
#include "Net/UnrealNetwork.h"
#include "WOG/GameInstance/WOGGameInstance.h"
#include "WOG/Lobby/WOGLobbyAvatar.h"
#include "WOG/Lobby/WOGLobbyPlayerSpot.h"
#include "GameFramework/PlayerState.h"
#include "GameMode/WOGLobbyGameMode.h"
#include "Kismet/GameplayStatics.h"


AWOGLobbyPlayerController::AWOGLobbyPlayerController()
{
	bReplicates = true;
	bAlwaysRelevant = true;
	
	bIsAttacker = false;
	bIsPlayerReady = false;
	LobbyAvatar = nullptr;
	LobbyPlayerSpot = nullptr;
}

void AWOGLobbyPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWOGLobbyPlayerController, bIsPlayerReady);
	DOREPLIFETIME(AWOGLobbyPlayerController, LobbyAvatar);
	DOREPLIFETIME(AWOGLobbyPlayerController, LobbyPlayerSpot);
	DOREPLIFETIME(AWOGLobbyPlayerController, bIsAttacker);
}

void AWOGLobbyPlayerController::OnNetCleanup(UNetConnection* Connection)
{
	if (GetLocalRole() == ROLE_Authority && PlayerState != NULL)
	{
		AWOGLobbyGameMode* GameMode = Cast<AWOGLobbyGameMode>(GetWorld()->GetAuthGameMode());
		if (GameMode)
		{
			GameMode->PreLogout(this);
		}
	}

	Super::OnNetCleanup(Connection);
}

void AWOGLobbyPlayerController::BeginPlay()
{
	Super::BeginPlay();
	FInputModeGameAndUI UIInput;
	SetInputMode(UIInput);
	SetShowMouseCursor(true);
}

void AWOGLobbyPlayerController::OnRep_IsPlayerReady()
{
	if(!LobbyAvatar) return;
	LobbyAvatar->UpdateIsPlayerReady(bIsPlayerReady);
}

void AWOGLobbyPlayerController::SetViewTargetCamera(AActor* NewViewTarget)
{
	SetViewTargetWithBlend(NewViewTarget, 0.35f);
}

void AWOGLobbyPlayerController::Server_SaveGame_Implementation()
{
	SaveGame();
}

void AWOGLobbyPlayerController::Server_ServerTravel_Implementation(const FString& Address)
{
	UWorld* World = GetWorld();
	if (World)
	{
		World->ServerTravel(Address, true);
	}
}

void AWOGLobbyPlayerController::SaveGame()
{
	if (LobbyAvatar)
	{
		PlayerProfile = LobbyAvatar->MeshProperties;
	}
	if (PlayerState)
	{
		PlayerProfile.PlayerName = PlayerState->GetPlayerName();
	}
	if (LobbyPlayerSpot)
	{
		PlayerProfile.UserIndex = LobbyPlayerSpot->UserIndex;
	}

	UWOGGameInstance* GameInstance = GetGameInstance<UWOGGameInstance>();
	if (GameInstance)
	{
		GameInstance->PlayersMap.Add(PlayerProfile.UserIndex, PlayerProfile.PlayerName);
	}

	UPlayerProfileSaveGame* SaveGameObject = Cast<UPlayerProfileSaveGame>(UGameplayStatics::CreateSaveGameObject(UPlayerProfileSaveGame::StaticClass()));
	if (SaveGameObject)
	{
		SaveGameObject->PlayerProfile = PlayerProfile;
		if (UGameplayStatics::SaveGameToSlot(SaveGameObject, PlayerProfile.PlayerName, PlayerProfile.UserIndex))
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Orange, FString::Printf(TEXT("Player Name: %s, PlayerIndex: %d"), *PlayerProfile.PlayerName, PlayerProfile.UserIndex));
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString("SaveGame failed"));
		}
	}
}

void AWOGLobbyPlayerController::Server_SetPlayerReady_Implementation(bool bPlayerReady)
{
	bIsPlayerReady = bPlayerReady;

	AWOGLobbyGameMode* GameMode = Cast<AWOGLobbyGameMode>(GetWorld()->GetAuthGameMode());
	if (!GameMode) return;

	if(bPlayerReady)
	{
		GameMode->IncreasePlayerReady();
	}
	else
	{
		GameMode->DecreasePlayerReady();
	}

	if(!LobbyAvatar) return;
	LobbyAvatar->bIsPlayerReady = bIsPlayerReady;
	LobbyAvatar->UpdateIsPlayerReady(bIsPlayerReady);
}

void AWOGLobbyPlayerController::Server_SetIsAttacker_Implementation(bool NewIsAttacker)
{
	bIsAttacker = NewIsAttacker;
	SetIsAttacker(bIsAttacker);
}

void AWOGLobbyPlayerController::OnRep_IsAttacker()
{
	SetIsAttacker(bIsAttacker);
}

void AWOGLobbyPlayerController::SetIsAttacker(bool NewIsAttacker)
{
	bIsAttacker = NewIsAttacker;
}
