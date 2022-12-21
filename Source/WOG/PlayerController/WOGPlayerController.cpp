// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGPlayerController.h"
#include "WOG/GameInstance/WOGGameInstance.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "WOG/Data/PlayerProfileSaveGame.h"
#include "WOG/PlayerCharacter/BasePlayerCharacter.h"

void AWOGPlayerController::OnPossess(APawn* aPawn)
{
	Super::OnPossess(aPawn);

	UWOGGameInstance* GameInstance = GetGameInstance<UWOGGameInstance>();
	if (ensureMsgf(!GameInstance, TEXT("Invalid game instance!"))) return;

	TArray <FString> PlayerNameArray;
	GameInstance->PlayersMap.GenerateValueArray(PlayerNameArray);
	FString DesiredPlayerName = PlayerState->GetPlayerName();
	UPlayerProfileSaveGame* WOGSavegame = nullptr;

	if (ensureMsgf(PlayerNameArray.IsEmpty(), TEXT("PlayerNameArrayEmpty"))) return;

	for (int32 i = 0; i < PlayerNameArray.Num(); i++)
	{
		FString PlayerName = PlayerNameArray[i];
		if (PlayerName != DesiredPlayerName) continue;

		WOGSavegame = Cast<UPlayerProfileSaveGame>(UGameplayStatics::LoadGameFromSlot(PlayerName, i));
	}

	ABasePlayerCharacter* PlayerCharacter = Cast<ABasePlayerCharacter>(aPawn);
	if (ensureMsgf((!WOGSavegame || !PlayerCharacter), TEXT("WOGSaveGame or PlayerCharacter invalid"))) return;

	PlayerCharacter->PlayerProfile = WOGSavegame->PlayerProfile;
	PlayerCharacter->Server_SetPlayerProfile(WOGSavegame->PlayerProfile);

	Server_SetPlayerIndex(WOGSavegame->PlayerProfile.UserIndex);
}

void AWOGPlayerController::Server_SetPlayerIndex_Implementation(int32 NewIndex)
{
	UserIndex = NewIndex;
}
