// Fill out your copyright notice in the Description page of Project Settings.


#include "WOG_HUD.h"
#include "WOG/PlayerCharacter/BasePlayerCharacter.h"
#include "WOG/PlayerController/WOGPlayerController.h"
#include "WOG/ActorComponents/WOGAttributesComponent.h"

void UWOG_HUD::NativeOnInitialized()
{
    OwnerPlayerController = Cast<AWOGPlayerController>(GetOwningPlayer());
    if (OwnerPlayerController)
    {
        OwnerPlayerCharacter = Cast<ABasePlayerCharacter>(OwnerPlayerController->GetCharacter());
    }
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Orange, FString("HUD initialized"));
    }

    Super::NativeOnInitialized();
}

void UWOG_HUD::ResetHUDAfterRespawn()
{
    OwnerPlayerController = OwnerPlayerController == nullptr ? Cast<AWOGPlayerController>(GetOwningPlayer()) : OwnerPlayerController;
    if (OwnerPlayerController)
    {
        OwnerPlayerCharacter = OwnerPlayerCharacter == nullptr ? Cast<ABasePlayerCharacter>(OwnerPlayerController->GetCharacter()) : OwnerPlayerCharacter;
    }

}

float UWOG_HUD::GetHealthPercent()
{
    if (OwnerPlayerCharacter && OwnerPlayerCharacter->GetAttributes())
    {
        return OwnerPlayerCharacter->GetAttributes()->GetHealthPercent();
    }
    else
    {
        GEngine->AddOnScreenDebugMessage(1, 2.f, FColor::Red, FString("Controller Error"));
        OwnerPlayerController = OwnerPlayerController == nullptr ? Cast<AWOGPlayerController>(GetOwningPlayer()) : OwnerPlayerController;
        OwnerPlayerCharacter = Cast<ABasePlayerCharacter>(OwnerPlayerController->K2_GetPawn());
        if (OwnerPlayerCharacter && OwnerPlayerCharacter->GetAttributes())
        {
            return OwnerPlayerCharacter->GetAttributes()->GetHealthPercent();
        }
        else
        {
            GEngine->AddOnScreenDebugMessage(1, 2.f, FColor::Red, FString("Pawn Error"));
            return 0.f;
        }
    }
}

float UWOG_HUD::GetManaPercent()
{
    if (OwnerPlayerCharacter && OwnerPlayerCharacter->GetAttributes())
    {
        return OwnerPlayerCharacter->GetAttributes()->GetManaPercent();
    }
    else
    {
        OwnerPlayerController = OwnerPlayerController == nullptr ? Cast<AWOGPlayerController>(GetOwningPlayer()) : OwnerPlayerController;
        OwnerPlayerCharacter = Cast<ABasePlayerCharacter>(OwnerPlayerController->K2_GetPawn());
        if (OwnerPlayerCharacter && OwnerPlayerCharacter->GetAttributes())
        {
            return OwnerPlayerCharacter->GetAttributes()->GetManaPercent();
        }
        else
        {
            return 0.f;
        }
    }
}

float UWOG_HUD::GetAdrenalinePercent()
{
    if (OwnerPlayerCharacter && OwnerPlayerCharacter->GetAttributes())
    {
        return OwnerPlayerCharacter->GetAttributes()->GetAdrenalinePercent();
    }
    else
    {
        OwnerPlayerController = OwnerPlayerController == nullptr ? Cast<AWOGPlayerController>(GetOwningPlayer()) : OwnerPlayerController;
        OwnerPlayerCharacter = Cast<ABasePlayerCharacter>(OwnerPlayerController->K2_GetPawn());
        if (OwnerPlayerCharacter && OwnerPlayerCharacter->GetAttributes())
        {
            return OwnerPlayerCharacter->GetAttributes()->GetAdrenalinePercent();
        }
        else
        {
            return 0.f;
        }
    }
}
