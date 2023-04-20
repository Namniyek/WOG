// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGPlayerController.h"
#include "WOG/GameInstance/WOGGameInstance.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "WOG/Data/PlayerProfileSaveGame.h"
#include "WOG/PlayerCharacter/BasePlayerCharacter.h"
#include "WOG/UI/WOGMatchHUD.h"
#include "Blueprint/UserWidget.h"
#include "WOG/UI/MainAnnouncementWidget.h"
#include "WOG/Data/TODEnum.h"
#include "WOG/UI/EndgameWidget.h"
#include "Net/UnrealNetwork.h"
#include "WOG/ActorComponents/WOGCombatComponent.h"
#include "WOG/Enemies/WOGPossessableEnemy.h"


void AWOGPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWOGPlayerController, bIsAttacker);
}

void AWOGPlayerController::Test(APawn* NewPawn)
{
	if (NewPawn)
	{
		if (HasAuthority())
		{
			UE_LOG(LogTemp, Error, TEXT("SERVER - Pawn: %s"), *NewPawn->GetName());
		}
		if (!HasAuthority())
		{
			UE_LOG(LogTemp, Error, TEXT("CLIENT - Pawn: %s"), *NewPawn->GetName());
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Pawn invalid"));
	}
}

void AWOGPlayerController::OnPossess(APawn* aPawn)
{
	Super::OnPossess(aPawn);

	if (DefaultPawn) return;
	DefaultPawn = Cast<ABasePlayerCharacter>(aPawn);

	UWOGGameInstance* GameInstance = GetGameInstance<UWOGGameInstance>();
	if (!GameInstance)
	{
		return;
	}

	TArray <FString> PlayerNameArray;
	GameInstance->PlayersMap.GenerateValueArray(PlayerNameArray);
	FString DesiredPlayerName = PlayerState->GetPlayerName();
	UPlayerProfileSaveGame* WOGSavegame = nullptr;

	if (PlayerNameArray.IsEmpty()) return;

	for (int32 i = 0; i < PlayerNameArray.Num(); i++)
	{
		FString PlayerName = PlayerNameArray[i];
		if (PlayerName != DesiredPlayerName) continue;

		WOGSavegame = Cast<UPlayerProfileSaveGame>(UGameplayStatics::LoadGameFromSlot(PlayerName, i));
	}

	ABasePlayerCharacter* PlayerCharacter = Cast<ABasePlayerCharacter>(aPawn);
	if (!WOGSavegame || !PlayerCharacter) return;

	PlayerCharacter->Server_SetPlayerProfile(WOGSavegame->PlayerProfile);
	bIsAttacker = PlayerCharacter->PlayerProfile.bIsAttacker;
	SetPawn(PlayerCharacter);
	PlayerCharacter->SetOwnerPC(this);

	Server_SetPlayerIndex(WOGSavegame->PlayerProfile.UserIndex);

	Client_ResetHUD();

	PlayerCharacter->GetCombatComponent()->Server_CreateMainWeapon(PlayerCharacter->GetCombatComponent()->GetDefaultWeaponClass());
}

void AWOGPlayerController::BeginPlay()
{
	Super::BeginPlay();
	MatchHUD = Cast<AWOGMatchHUD>(GetHUD());
}

void AWOGPlayerController::PossessMinion_Implementation(AActor* ActorToPossess)
{
	if (!ActorToPossess)
	{
		UE_LOG(LogTemp, Error, TEXT("ActorToPossess invalid"));
		return;
	}

	TObjectPtr<APawn> PawnToPossess = Cast<APawn>(ActorToPossess);
	if (!PawnToPossess)
	{
		UE_LOG(LogTemp, Error, TEXT("PawnToPossess invalid"));
		return;
	}

	if (PawnToPossess->IsPlayerControlled())
	{
		UE_LOG(LogTemp, Error, TEXT("ALREADY CONTROLLED BY ANOTHER PLAYER"));
		return;
	}

	FTimerHandle BlendTimer;
	FTimerDelegate BlendDelegate;
	BlendDelegate.BindUFunction(this, FName("Possess"), PawnToPossess);
	float BlendTime = 1.f;
	GetWorldTimerManager().SetTimer(BlendTimer, BlendDelegate, BlendTime, false);

	if (ActorToPossess)
	{
		SetViewTargetWithBlend(ActorToPossess, BlendTime);
	}
}

void AWOGPlayerController::UnpossessMinion_Implementation()
{
	if (!DefaultPawn)
	{
		UE_LOG(LogTemp, Error, TEXT("DefaultPawn invalid"));
		return;
	}

	TObjectPtr<AActor> ActorToPossess = Cast<AActor>(DefaultPawn);
	if (!ActorToPossess)
	{
		UE_LOG(LogTemp, Error, TEXT("ActorToPossess invalid"));
		return;
	}

	TObjectPtr<APawn> PawnToPossess = Cast<APawn>(DefaultPawn);
	if (!PawnToPossess)
	{
		UE_LOG(LogTemp, Error, TEXT("PawnToPossess invalid"));
		return;
	}

	UnPossess();
	Possess(PawnToPossess);
}

void AWOGPlayerController::Server_SetPlayerIndex_Implementation(int32 NewIndex)
{
	UserIndex = NewIndex;
}

void AWOGPlayerController::Client_CreateAnnouncementWidget_Implementation(ETimeOfDay NewTOD)
{
	MatchHUD == nullptr ? Cast<AWOGMatchHUD>(GetHUD()) : MatchHUD;
	if (!MatchHUD || !MatchHUD->AnnouncementClass) return;

	TOD = NewTOD;
	FString StringMain = FString();
	FString StringSec = FString();
	SetTODString(TOD, StringMain, StringSec);

	MatchHUD->AddAnnouncementWidget(StringMain, StringSec);
}

void AWOGPlayerController::SetTODString(ETimeOfDay CurrentTOD, FString& StringMain, FString& StringSec)
{
	switch (CurrentTOD)
	{
	case ETimeOfDay::TOD_Dusk1:
		StringMain = FString("Dusk of the first day");
		if (bIsAttacker)
		{
			StringSec = FString("Destroy the Village");
		}
		else
		{
			StringSec = FString("Defend the Village!");
		}
		break;
	case ETimeOfDay::TOD_Dawn2:
		StringMain = FString("Dawn of the second day");
		StringSec = FString("Rest and prepare for the night!");
		break;
	case ETimeOfDay::TOD_Dusk2:
		StringMain = FString("Dusk of the second day");
		if (bIsAttacker)
		{
			StringSec = FString("Destroy the Village");
		}
		else
		{
			StringSec = FString("Defend the Village!");
		}
		break;
	case ETimeOfDay::TOD_Dawn3:
		StringMain = FString("Dawn of the final day");
		StringSec = FString("Rest and prepare for the night!");
		break;
	case ETimeOfDay::TOD_Dusk3:
		StringMain = FString("Dusk of the final day");
		if (bIsAttacker)
		{
			StringSec = FString("Destroy the Village");
		}
		else
		{
			StringSec = FString("Defend the Village!");
		}
		break;
	case ETimeOfDay::TOD_Dawn4:
		StringMain = FString("Game Over!");
		StringSec = FString("");
		break;
	default:
		return;
	}
}

void AWOGPlayerController::Client_CreateEndgameWidget_Implementation()
{
	MatchHUD == nullptr ? Cast<AWOGMatchHUD>(GetHUD()) : MatchHUD;
	if (!MatchHUD) return;

	MatchHUD->AddEndgameWidget();	
}

void AWOGPlayerController::Client_ResetHUD_Implementation()
{
	MatchHUD == nullptr ? Cast<AWOGMatchHUD>(GetHUD()) : MatchHUD;
	if (MatchHUD)
	{
		MatchHUD->ResetHUDAfterRespawn();
	}
}
