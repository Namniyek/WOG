// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGGameMode.h"
#include "WOG/GameInstance/WOGGameInstance.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "WOG/PlayerCharacter/BasePlayerCharacter.h"
#include "WOG/PlayerController/WOGPlayerController.h"

void AWOGGameMode::BeginPlay()
{

}

void AWOGGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	GEngine->AddOnScreenDebugMessage(5, 5.f, FColor::Yellow, FString::Printf(TEXT("NewPlayerJoined")));
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);

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
	if (!GameInstance) return;

	TArray <FString> PlayerNameArray;
	GameInstance->PlayersMap.GenerateValueArray(PlayerNameArray);
	FString DesiredPlayerName = NewPlayer->PlayerState->GetPlayerName();

	if (PlayerNameArray.IsEmpty()) return;

	for (int32 i = 0; i < PlayerNameArray.Num(); i++)
	{
		FString PlayerName = PlayerNameArray[i];
		if (PlayerName != DesiredPlayerName) continue;

		if (i <= 2)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = NewPlayer;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
			ABasePlayerCharacter* Defender = 
				GetWorld()->SpawnActor<ABasePlayerCharacter>(DefenderCharacter, GetPlayerStart(FString::FromInt(i)), SpawnParams);
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("UserIndex: %d"), i));
			UE_LOG(LogTemp, Warning, TEXT("UserIndex: %d"), i);
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
				GetWorld()->SpawnActor<ABasePlayerCharacter>(AttackerCharacter, GetPlayerStart(FString::FromInt(i)), SpawnParams);
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("UserIndex: %d"), i));
			UE_LOG(LogTemp, Warning, TEXT("UserIndex: %d"), i);
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
	if (!GameInstance) return;

	TArray <FString> PlayerNameArray;
	GameInstance->PlayersMap.GenerateValueArray(PlayerNameArray);
	FString DesiredPlayerName = NewPlayer->PlayerState->GetPlayerName();

	if (PlayerNameArray.IsEmpty()) return;

	for (int32 i = 0; i < PlayerNameArray.Num(); i++)
	{
		FString PlayerName = PlayerNameArray[i];
		if (PlayerName == FString("empty"))
		{
			GameInstance->PlayersMap.Add(i, NewPlayer->PlayerState->GetPlayerName());
			UPlayerProfileSaveGame* SaveGameObject = Cast<UPlayerProfileSaveGame>(UGameplayStatics::CreateSaveGameObject(UPlayerProfileSaveGame::StaticClass()));
			if (SaveGameObject)
			{
				SaveGameObject->PlayerProfile.bIsAttacker = (i > 2);
				SaveGameObject->PlayerProfile.bIsMale = FMath::RandBool();
				SaveGameObject->PlayerProfile.BodyPaintColor = "0";
				SaveGameObject->PlayerProfile.CharacterIndex = "0";
				SaveGameObject->PlayerProfile.HairColor = "0";
				SaveGameObject->PlayerProfile.PrimaryColor = "0";
				SaveGameObject->PlayerProfile.Rune = "0";
				SaveGameObject->PlayerProfile.SkinColor = "0";
				SaveGameObject->PlayerProfile.UserIndex = i;
				SaveGameObject->PlayerProfile.PlayerName = NewPlayer->PlayerState->GetPlayerName();

				UGameplayStatics::SaveGameToSlot(SaveGameObject, SaveGameObject->PlayerProfile.PlayerName, SaveGameObject->PlayerProfile.UserIndex);
			}
		}
		return;
	}
	return;
}

FTransform AWOGGameMode::GetPlayerStart(FString StartIndex)
{
	FName StartIndexName = (*StartIndex);
	GEngine->AddOnScreenDebugMessage(5, 5.f, FColor::Yellow, StartIndex);
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

	GameInstance = GetGameInstance<UWOGGameInstance>();
	if (!GameInstance) return;

	AWOGPlayerController* PlayerController = Cast<AWOGPlayerController>(Exiting);
	if (PlayerController)
	{
		GameInstance->PlayersMap.Add(PlayerController->UserIndex, FString("empty"));
	}

	bHandleDropIn = true;
}
