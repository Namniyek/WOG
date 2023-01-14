// Fill out your copyright notice in the Description page of Project Settings.


#include "EndgameWidget.h"
#include "WOG/GameState/WOGGameState.h"
#include "WOG/PlayerController/WOGPlayerController.h"
#include "Components/TextBlock.h"

void UEndgameWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();
    SetResults();
    
}

FPlayerStats UEndgameWidget::GetPlayerStats()
{

    PlayerState = CastChecked<AWOGPlayerState>(GetOwningPlayerState());

    if(PlayerState)
    {
        return PlayerState->GetPlayerStats();
    }
    return FPlayerStats();
}

FString UEndgameWidget::GetWinnerTeam()
{
    UWorld* World = GetWorld();
    if (World)
    {
        GameState = World->GetGameState<AWOGGameState>();
        if (GameState)
        {
            if (GameState->IsWinnerAttacker())
            {
                return FString("Attackers won!");
            }
            else
            {
                return FString("Defenders won!");
            }
        }
    }
    return FString("ERROR");
}

void UEndgameWidget::SetResults()
{
    ResultText->SetText(FText::FromString(GetWinnerTeam()));

    PlayerStats = GetPlayerStats();
    FString LocalTimesElimmed = FString::FromInt(PlayerStats.TimesElimmed);
    FString LocalTotalElimms = FString::FromInt(PlayerStats.TotalElimms);

    TimesElimmed->SetText(FText::FromString(LocalTimesElimmed));
    TotalElimms->SetText(FText::FromString(LocalTotalElimms));
    MostElimmedPlayer->SetText(FText::FromString(PlayerStats.MostElimmedPlayer));
}
