// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGGameState.h"
#include "Engine/NetConnection.h"
#include "GameFramework/PlayerState.h"
#include "WOG/PlayerController/WOGPlayerController.h"
#include "GameFramework/HUD.h"
#include "GameFramework/GameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "WOG/PlayerState/WOGPlayerState.h"


void AWOGGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWOGGameState, bAttackersWon);
	DOREPLIFETIME(AWOGGameState, MostElimmedPlayer);
	DOREPLIFETIME(AWOGGameState, PlayerWithMostElimms);
}

void AWOGGameState::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();
	SetupTOD();

	switch (FinishMatchTOD)
	{
	case ETimeOfDay::TOD_Dawn2:
		FinishMatchDayNumber = 2;
		break;
	case ETimeOfDay::TOD_Dawn3:
		FinishMatchDayNumber = 3;
		break;
	case ETimeOfDay::TOD_Dawn4:
		FinishMatchDayNumber = 4;
		break;
	default:
		break;
	}
}

void AWOGGameState::SetupTOD()
{
	TODActor = Cast<ATimeOfDay>(UGameplayStatics::GetActorOfClass(this, TimeOfDayClass));
	if (TODActor)
	{
		TODActor->TimeOfDayChanged.AddDynamic(this, &ThisClass::TimeOfDayChanged);
		TODActor->DayChanged.AddDynamic(this, &ThisClass::DayChanged);
	}
}

void AWOGGameState::TimeOfDayChanged(ETimeOfDay TOD)
{
	if (TOD != FinishMatchTOD)
	{
		HandleTODAnnouncement(TOD);
	}
	else
	{
		//Do not create announcement at the last TOD
	}
}

void AWOGGameState::DayChanged(int32 DayNumber)
{
	if (DayNumber == FinishMatchDayNumber)
	{
		Server_HandleEndGame();
	}
}

void AWOGGameState::HandleTODAnnouncement(ETimeOfDay TOD)
{
	for (auto PlayerState : PlayerArray)
	{
		if (PlayerState)
		{
			AWOGPlayerController* PC = Cast<AWOGPlayerController>(PlayerState->GetPlayerController());
			if (PC)
			{
				PC->Client_CreateAnnouncementWidget(TOD);
			}
		}
	}
}

void AWOGGameState::Server_HandleEndGame_Implementation()
{
	if (TODActor)
	{
		TODActor->StopCycle();
		TODActor->DayChanged.RemoveDynamic(this, &ThisClass::DayChanged);
		TODActor->TimeOfDayChanged.RemoveDynamic(this, &ThisClass::TimeOfDayChanged);
		if (HasAuthority())
		{
			AGameMode* GameMode = CastChecked<AGameMode>(GetWorld()->GetAuthGameMode());
			GameMode->EndMatch();
		}
	}

	for (auto PlayerState : PlayerArray)
	{
		if (!PlayerState) break;

		AWOGPlayerState* WOGPlayerState = Cast<AWOGPlayerState>(PlayerState);
		if (!WOGPlayerState) break;

		AWOGPlayerController* PC = Cast<AWOGPlayerController>(WOGPlayerState->GetPlayerController());
		if (PC)
		{
			PC->Client_CreateEndgameWidget();
			if (PC->GetPawn())
			{
				PC->GetPawn()->Destroy();
			}
		}
	}
}

void AWOGGameState::Server_SetEndgamePlayerStats_Implementation()
{
	Multicast_SetEndgamePlayerStats();
}

void AWOGGameState::Multicast_SetEndgamePlayerStats_Implementation()
{
	SetEndgamePlayerStats();
}

void AWOGGameState::SetEndgamePlayerStats()
{
	for (auto PlayerState : PlayerArray)
	{
		if (!PlayerState) break;

		AWOGPlayerState* WOGPlayerState = Cast<AWOGPlayerState>(PlayerState);
		if (!WOGPlayerState) break;

		if (WOGPlayerState->GetPlayerStats().TimesElimmed > MostElimmed)
		{
			MostElimmedPlayer = WOGPlayerState->GetPlayerName();
			MostElimmed = WOGPlayerState->GetPlayerStats().TimesElimmed;
		}

		if (WOGPlayerState->GetPlayerStats().TotalElimms > MostElimms)
		{
			PlayerWithMostElimms = WOGPlayerState->GetPlayerName();
			MostElimms = WOGPlayerState->GetPlayerStats().TotalElimms;
		}

		WOGPlayerState->SetMostElimmedPlayer(MostElimmedPlayer);
		WOGPlayerState->SetPlayerWithMostElimms(PlayerWithMostElimms);
	}
}



