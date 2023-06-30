// Fill out your copyright notice in the Description page of Project Settings.


#include "WOG_HUD.h"
#include "WOG/PlayerCharacter/BasePlayerCharacter.h"
#include "WOG/PlayerController/WOGPlayerController.h"
#include "AbilitySystem/AttributeSets/WOGAttributeSetBase.h"

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
    OwnerPlayerController = OwnerPlayerController == nullptr ? Cast<AWOGPlayerController>(GetOwningPlayer()) : OwnerPlayerController;
    OwnerPlayerCharacter = Cast<ABasePlayerCharacter>(OwnerPlayerController->K2_GetPawn());
    if (OwnerPlayerCharacter && OwnerPlayerCharacter->GetAttributeSetBase())
    {
        return OwnerPlayerCharacter->GetAttributeSetBase()->GetHealth() / OwnerPlayerCharacter->GetAttributeSetBase()->GetMaxHealth();
    }
    else
    {
        GEngine->AddOnScreenDebugMessage(1, 2.f, FColor::Red, FString("Pawn Error"));
        return 0.f;
    }
}

float UWOG_HUD::GetManaPercent()
{
    OwnerPlayerController = OwnerPlayerController == nullptr ? Cast<AWOGPlayerController>(GetOwningPlayer()) : OwnerPlayerController;
    OwnerPlayerCharacter = Cast<ABasePlayerCharacter>(OwnerPlayerController->K2_GetPawn());
    if (OwnerPlayerCharacter && OwnerPlayerCharacter->GetAttributeSetBase())
    {
        return OwnerPlayerCharacter->GetAttributeSetBase()->GetMana() / OwnerPlayerCharacter->GetAttributeSetBase()->GetMaxMana();
    }
    else
    {
        GEngine->AddOnScreenDebugMessage(1, 2.f, FColor::Red, FString("Pawn Error"));
        return 0.f;
    }
}

float UWOG_HUD::GetAdrenalinePercent()
{
    OwnerPlayerController = OwnerPlayerController == nullptr ? Cast<AWOGPlayerController>(GetOwningPlayer()) : OwnerPlayerController;
    OwnerPlayerCharacter = Cast<ABasePlayerCharacter>(OwnerPlayerController->K2_GetPawn());
    if (OwnerPlayerCharacter && OwnerPlayerCharacter->GetAttributeSetBase())
    {
        return OwnerPlayerCharacter->GetAttributeSetBase()->GetAdrenaline() / OwnerPlayerCharacter->GetAttributeSetBase()->GetMaxAdrenaline();
    }
    else
    {
        GEngine->AddOnScreenDebugMessage(1, 2.f, FColor::Red, FString("Pawn Error"));
        return 0.f;
    }
}

float UWOG_HUD::GetStaminaPercent()
{
    OwnerPlayerController = OwnerPlayerController == nullptr ? Cast<AWOGPlayerController>(GetOwningPlayer()) : OwnerPlayerController;
    OwnerPlayerCharacter = Cast<ABasePlayerCharacter>(OwnerPlayerController->K2_GetPawn());
    if (OwnerPlayerCharacter && OwnerPlayerCharacter->GetAttributeSetBase())
    {
        return OwnerPlayerCharacter->GetAttributeSetBase()->GetStamina() / OwnerPlayerCharacter->GetAttributeSetBase()->GetMaxStamina();
    }
    else
    {
        GEngine->AddOnScreenDebugMessage(1, 2.f, FColor::Red, FString("Pawn Error"));
        return 0.f;
    }
}
