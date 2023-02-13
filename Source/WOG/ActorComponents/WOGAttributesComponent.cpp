// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGAttributesComponent.h"
#include "Net/UnrealNetwork.h"
#include "WOG/PlayerCharacter/BasePlayerCharacter.h"
#include "WOG/GameMode/WOGGameMode.h"
#include "WOG/PlayerController/WOGPlayerController.h"

UWOGAttributesComponent::UWOGAttributesComponent()
{
	MaxHealth = 100.f;
	Health = 100.f;
	MaxMana = 100.f;
	Mana = 100.f;
	MaxAdrenaline = 100.f;
	Adrenaline = 0.f;

}

void UWOGAttributesComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UWOGAttributesComponent, MaxHealth);
	DOREPLIFETIME(UWOGAttributesComponent, Health);
	DOREPLIFETIME(UWOGAttributesComponent, HealthPercent);
	DOREPLIFETIME(UWOGAttributesComponent, MaxMana);
	DOREPLIFETIME(UWOGAttributesComponent, Mana);
	DOREPLIFETIME(UWOGAttributesComponent, MaxAdrenaline);
	DOREPLIFETIME(UWOGAttributesComponent, Adrenaline);
}

void UWOGAttributesComponent::OnRep_Health()
{
}

void UWOGAttributesComponent::OnRep_Mana()
{
}

void UWOGAttributesComponent::OnRep_Adrenaline()
{
}

void UWOGAttributesComponent::Server_UpdateHealth_Implementation(float Value, AController* InstigatedBy)
{
	UpdateHealth(Value, InstigatedBy);
}

void UWOGAttributesComponent::UpdateHealth(float Value, AController* InstigatedBy)
{
	OwnerCharacter = OwnerCharacter == nullptr ? Cast<ABasePlayerCharacter>(GetOwner()) : OwnerCharacter;
	if (!OwnerCharacter || OwnerCharacter->CharacterState == ECharacterState::ECS_Elimmed) return;

	if (Health + Value >= MaxHealth)
	{
		Health = MaxHealth;
	}
	else if (Health + Value <= 0.f)
	{
		Health = 0.f;

		OwnerCharacter = OwnerCharacter == nullptr ? Cast<ABasePlayerCharacter>(GetOwner()) : OwnerCharacter;
		if (!OwnerCharacter) return;

		OwnerCharacter->WOGGameMode = OwnerCharacter->WOGGameMode == nullptr ?
			GetWorld()->GetAuthGameMode<AWOGGameMode>() : OwnerCharacter->WOGGameMode;

		OwnerPC = OwnerPC ==nullptr ? Cast<AWOGPlayerController>(OwnerCharacter->GetController()) : OwnerPC;
		AWOGPlayerController* Attacker = Cast<AWOGPlayerController>(InstigatedBy);
		if (!OwnerCharacter->WOGGameMode || !OwnerPC || !Attacker) return;

		OwnerCharacter->Server_SetCharacterState(ECharacterState::ECS_Elimmed);
		OwnerCharacter->WOGGameMode->PlayerEliminated(OwnerCharacter, OwnerPC, Attacker);
	}
	else
	{
		Health = Health + Value;
	}

	HealthPercent = Health / MaxHealth;
}

void UWOGAttributesComponent::UpdateMana(float Value)
{
}

void UWOGAttributesComponent::UpdateAdranaline(float Value)
{
}
