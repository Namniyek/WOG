// Fill out your copyright notice in the Description page of Project Settings.


#include "EndgameWidget.h"

#include "CommonTextBlock.h"
#include "WOG/GameState/WOGGameState.h"
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

bool UEndgameWidget::IsWinnerAttacker()
{
    UWorld* World = GetWorld();
    if (World)
    {
        GameState = World->GetGameState<AWOGGameState>();
        if (GameState)
        {
            return GameState->IsWinnerAttacker();
        }
    }
    return false;
}

void UEndgameWidget::SetResults()
{
    PlayerStats = GetPlayerStats();
    const FString LocalTimesElimmed = FString::FromInt(PlayerStats.TimesElimmed);
    const FString LocalTotalElimms = FString::FromInt(PlayerStats.TotalElimms);

    TimesElimmed->SetText(FText::FromString(LocalTimesElimmed));
    TotalElimms->SetText(FText::FromString(LocalTotalElimms));
    MostElimmedPlayer->SetText(FText::FromString(PlayerStats.MostElimmedPlayer));
    PlayerWithMostElimms->SetText(FText::FromString(PlayerStats.PlayerWithMostElimms));
}
