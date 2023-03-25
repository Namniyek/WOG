// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGAttributesComponent.h"
#include "Net/UnrealNetwork.h"
#include "WOG/PlayerCharacter/BasePlayerCharacter.h"
#include "WOG/GameMode/WOGGameMode.h"
#include "WOG/PlayerController/WOGPlayerController.h"

UWOGAttributesComponent::UWOGAttributesComponent()
{
	MaxHealth = 100.f;
	Health = MaxHealth;
	MaxMana = 100.f;
	Mana = MaxMana;
	MaxAdrenaline = 100.f;
	Adrenaline = MaxAdrenaline;
	HealthPercent = Health / MaxHealth;
	ManaPercent = Mana / MaxMana;
	AdrenalinePercent = Adrenaline / MaxAdrenaline;

}

void UWOGAttributesComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UWOGAttributesComponent, MaxHealth);
	DOREPLIFETIME(UWOGAttributesComponent, Health);
	DOREPLIFETIME(UWOGAttributesComponent, HealthPercent);
	DOREPLIFETIME(UWOGAttributesComponent, MaxMana);
	DOREPLIFETIME(UWOGAttributesComponent, Mana);
	DOREPLIFETIME(UWOGAttributesComponent, ManaPercent);
	DOREPLIFETIME(UWOGAttributesComponent, MaxAdrenaline);
	DOREPLIFETIME(UWOGAttributesComponent, Adrenaline);
	DOREPLIFETIME(UWOGAttributesComponent, AdrenalinePercent);
}

void UWOGAttributesComponent::BeginPlay()
{
	Server_InitStats();
}

void UWOGAttributesComponent::Server_InitStats_Implementation()
{
	Health = MaxHealth;
	Mana = MaxMana;
	Adrenaline = MaxAdrenaline;
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
		OwnerCharacter = OwnerCharacter == nullptr ? Cast<ABasePlayerCharacter>(GetOwner()) : OwnerCharacter;
		if (!OwnerCharacter) return;

		OwnerCharacter->WOGGameMode = OwnerCharacter->WOGGameMode == nullptr ?
			GetWorld()->GetAuthGameMode<AWOGGameMode>() : OwnerCharacter->WOGGameMode;

		OwnerPC = OwnerPC ==nullptr ? Cast<AWOGPlayerController>(OwnerCharacter->GetController()) : OwnerPC;
		AWOGPlayerController* Attacker = Cast<AWOGPlayerController>(InstigatedBy);
		if (!OwnerCharacter->WOGGameMode || !OwnerPC || !Attacker)
		{
			if (!OwnerCharacter->WOGGameMode)
			{
				GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, FString("NoGameMode"));
			}
			if (!OwnerPC)
			{
				GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, FString("NoOwnerPC"));
			}
			if (!Attacker)
			{
				GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, FString("NoAttackerPC"));
			}
			return;
		}
		
		OwnerCharacter->WOGGameMode->PlayerEliminated(OwnerCharacter, OwnerPC, Attacker);

		Health = 0.f;
		OwnerCharacter->Server_SetCharacterState(ECharacterState::ECS_Elimmed);
	}
	else
	{
		Health += Value;
	}

	HealthPercent = Health / MaxHealth;
}

void UWOGAttributesComponent::Server_UpdateMana_Implementation(float Value)
{
	UpdateMana(Value);
}

void UWOGAttributesComponent::UpdateMana(float Value)
{
	OwnerCharacter = OwnerCharacter == nullptr ? Cast<ABasePlayerCharacter>(GetOwner()) : OwnerCharacter;
	if (!OwnerCharacter || OwnerCharacter->CharacterState == ECharacterState::ECS_Elimmed) return;

	if (Mana + Value >= MaxMana)
	{
		Mana = MaxMana;
	}
	else if (Mana + Value <= 0)
	{
		Mana = 0.f;
	}
	else
	{
		Mana += Value;
	}
	ManaPercent = Mana / MaxMana;
}

void UWOGAttributesComponent::Server_UpdateAdrenaline_Implementation(float Value)
{
	UpdateAdranaline(Value);
}

void UWOGAttributesComponent::UpdateAdranaline(float Value)
{
	OwnerCharacter = OwnerCharacter == nullptr ? Cast<ABasePlayerCharacter>(GetOwner()) : OwnerCharacter;
	if (!OwnerCharacter || OwnerCharacter->CharacterState == ECharacterState::ECS_Elimmed) return;

	if (Adrenaline + Value >= MaxAdrenaline)
	{
		Adrenaline = MaxAdrenaline;
	}
	else if (Adrenaline + Value <= 0)
	{
		Adrenaline = 0.f;
	}
	else
	{
		Adrenaline += Value;
	}
	AdrenalinePercent = Adrenaline / MaxAdrenaline;
}



void UWOGAttributesComponent::Server_PassiveAttributeUpdate_Implementation(EAttributeType AttributeToUpdate, float Value)
{
	OwnerCharacter = OwnerCharacter == nullptr ? Cast<ABasePlayerCharacter>(GetOwner()) : OwnerCharacter;
	if (!OwnerCharacter) return;

	AController* Controller = OwnerCharacter->GetController();

	switch (AttributeToUpdate)
	{
	case EAttributeType::AT_Health:

		if (!Controller) return;

		PassiveUpdateTimerDelegate.BindUFunction(this, FName("UpdateHealth"), Value, Controller);
		GetWorld()->GetTimerManager().SetTimer(PassiveUpdateTimer, PassiveUpdateTimerDelegate, UpdateFrequency, true);

		break;

	case EAttributeType::AT_Mana:

		PassiveUpdateTimerDelegate.BindUFunction(this, FName("UpdateMana"), Value);
		GetWorld()->GetTimerManager().SetTimer(PassiveUpdateTimer, PassiveUpdateTimerDelegate, UpdateFrequency, true);

		break;

	case EAttributeType::AT_Adrenaline:

		PassiveUpdateTimerDelegate.BindUFunction(this, FName("UpdateAdrenaline"), Value);
		GetWorld()->GetTimerManager().SetTimer(PassiveUpdateTimer, PassiveUpdateTimerDelegate, UpdateFrequency, true);

		break;
	}
}
