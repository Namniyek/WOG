// Fill out your copyright notice in the Description page of Project Settings.


#include "WOG_HUD.h"
#include "WOG/PlayerCharacter/BasePlayerCharacter.h"
#include "WOG/PlayerController/WOGPlayerController.h"

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
