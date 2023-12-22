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
#include "GameMode/WOGGameMode.h"
#include "PlayerCharacter//WOGAttacker.h"
#include "Libraries/WOGBlueprintLibrary.h"
#include "ActorComponents/WOGUIManagerComponent.h"
#include "Target/WOGBaseTarget.h"
#include "WOG.h"

AWOGGameState::AWOGGameState()
{
	CurrentTargetScore = 0;
	TotalTargetScore = 0;
	EndGameDelay = 5.f;
}

void AWOGGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWOGGameState, bAttackersWon);
	DOREPLIFETIME(AWOGGameState, MostElimmedPlayer);
	DOREPLIFETIME(AWOGGameState, PlayerWithMostElimms);
	DOREPLIFETIME(AWOGGameState, CurrentTargetScore);
	DOREPLIFETIME(AWOGGameState, TotalTargetScore);
}

void AWOGGameState::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();
	SetupTOD();
	InitTargetScores();

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
	//Handle endgame
	if (TOD != FinishMatchTOD)
	{
		HandleTODAnnouncement(TOD);
	}

	//Handle teleporting Attackers to base
	if (TOD == ETimeOfDay::TOD_Dawn2 || TOD == ETimeOfDay::TOD_Dawn3)
	{
		if (!HasAuthority()) return;
		for (auto Player : PlayerArray)
		{
			if (Player && Player->GetPlayerController())
			{
				AWOGAttacker* Attacker = Cast<AWOGAttacker>(Player->GetPawn());
				if (Attacker)
				{
					Attacker->Server_StartTeleportCharacter(GetPlayerStartTransform(Player->GetPlayerController()));
				}
			}
		}
	}
}

void AWOGGameState::InitTargetScores()
{
	if (!HasAuthority()) return;

	TArray<AActor*> OutTargets;
	UGameplayStatics::GetAllActorsOfClass(this, AWOGBaseTarget::StaticClass(), OutTargets);

	if (OutTargets.IsEmpty())
	{
		UE_LOG(WOGLogWorld, Error, TEXT("No Targets found in level"));
		return;
	}

	for (AActor* TargetActor : OutTargets)
	{
		if (!TargetActor) continue;

		AWOGBaseTarget* Target = Cast<AWOGBaseTarget>(TargetActor);
		if (!Target) continue;

		TotalTargetScore = TotalTargetScore + Target->GetTargetScore();
	}

	CurrentTargetScore = TotalTargetScore;
	UE_LOG(WOGLogWorld, Display, TEXT("TotalTargetScore: %d"), TotalTargetScore);
	UE_LOG(WOGLogWorld, Display, TEXT("CurrentTargetScore: %d"), CurrentTargetScore);
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
		if (!PlayerState) continue;

		APlayerController* PC = Cast<AWOGPlayerController>(PlayerState->GetPlayerController());
		if (!PC) continue;

		TObjectPtr<UWOGUIManagerComponent> UIManager = UWOGBlueprintLibrary::GetUIManagerComponent(PC);
		if (!UIManager) continue;
		UIManager->Client_AddAnnouncementWidget(TOD);
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
		if (!PlayerState) continue;

		APlayerController* PC = Cast<AWOGPlayerController>(PlayerState->GetPlayerController());
		if (!PC) continue;

		TObjectPtr<UWOGUIManagerComponent> UIManager = UWOGBlueprintLibrary::GetUIManagerComponent(PC);
		if (!UIManager) continue;
		UIManager->Client_AddEndgameWidget();

		if(PC->GetPawn())
		{
			PC->GetPawn()->Destroy();
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

FTransform AWOGGameState::GetPlayerStartTransform(APlayerController* PlayerController)
{
	AWOGPlayerController* PC = CastChecked<AWOGPlayerController>(PlayerController);
	AWOGGameMode* GameMode = CastChecked<AWOGGameMode>(AuthorityGameMode);
	if (PC && GameMode)
	{
		return GameMode->GetPlayerStart(FString::FromInt(PC->UserIndex));
	}

	UE_LOG(LogTemp, Error, TEXT("No PC or GameMode when teleporting player from GameState at dawn"));
	return FTransform();
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

void AWOGGameState::SubtractFromCurrentTargetScore(const int32& ScoreToSubtract)
{
	if (!HasAuthority()) return;

	CurrentTargetScore = CurrentTargetScore - ScoreToSubtract;
	UE_LOG(WOGLogWorld, Display, TEXT("CurrentTargetScore: %d"), CurrentTargetScore);

	OnCurrentTargetScoreChangedDelegate.Broadcast();
	Multicast_UpdateCurrentTargetScore();

	if (CurrentTargetScore <= 0)
	{
		//Attackers have destroyed all the targets
		bAttackersWon = true;

		FTimerHandle EndGameTimerHandle;
		GetWorldTimerManager().SetTimer(EndGameTimerHandle, this, &ThisClass::Server_HandleEndGame, EndGameDelay);
	}
}

float AWOGGameState::GetTargetScorePercentage()
{
	return CurrentTargetScore/TotalTargetScore;
}

void AWOGGameState::Multicast_UpdateCurrentTargetScore_Implementation()
{
	if (HasAuthority()) return;
	
	OnCurrentTargetScoreChangedDelegate.Broadcast();
}



