// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGAbilitiesComponent.h"
#include "WOG/PlayerCharacter/BasePlayerCharacter.h"
#include "Net/UnrealNetwork.h"
#include "WOG/Abilities/WOGBaseAbility.h"

UWOGAbilitiesComponent::UWOGAbilitiesComponent()
{
	SetIsReplicated(true);
}

void UWOGAbilitiesComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UWOGAbilitiesComponent, EquippedAbility);
	DOREPLIFETIME(UWOGAbilitiesComponent, EquippedAbilityType);
}

void UWOGAbilitiesComponent::BeginPlay()
{
	Super::BeginPlay();
	OwnerPlayerCharacter = OwnerPlayerCharacter == nullptr ? Cast<ABasePlayerCharacter>(GetOwner()) : OwnerPlayerCharacter;
}

void UWOGAbilitiesComponent::EquipAbility(const int32 Index)
{
	OwnerPlayerCharacter = OwnerPlayerCharacter == nullptr ? Cast<ABasePlayerCharacter>(GetOwner()) : OwnerPlayerCharacter;
	if (!OwnerPlayerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("No owner character"));
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = OwnerPlayerCharacter == nullptr ? Cast<ABasePlayerCharacter>(GetOwner()) : OwnerPlayerCharacter;
	EquippedAbility = GetWorld()->SpawnActor<AWOGBaseAbility>(CurrentAbilities[Index], OwnerPlayerCharacter->GetActorTransform(), SpawnParams);
	if (!EquippedAbility)
	{
		UE_LOG(LogTemp, Error, TEXT("No valid  EquippedAbility"));
		return;
	}

	EquippedAbilityType = EquippedAbility->GetAbilityType();
	EquippedAbility->Server_Equip();
}

void UWOGAbilitiesComponent::UnequipAbility()
{
	if (!EquippedAbility)
	{
		UE_LOG(LogTemp, Error, TEXT("No valid  EquippedAbility"));
		return;
	}

	EquippedAbilityType = EAbilityType::EAT_Relax;
	EquippedAbility->Server_Unequip();
	EquippedAbility = nullptr;
}

void UWOGAbilitiesComponent::Server_EquipAbility_Implementation(const int32 Index)
{
	if (!CurrentAbilities[Index])
	{
		UE_LOG(LogTemp, Error, TEXT("No valid ability"));
		return;
	}

	if (EquippedAbility && EquippedAbility->IsA(CurrentAbilities[Index]))
	{
		UE_LOG(LogTemp, Error, TEXT("EquippedAbility already of the desired type. Unequipping"));
		UnequipAbility();
		return;
	}

	if (EquippedAbility && !EquippedAbility->IsA(CurrentAbilities[Index]))
	{
		UE_LOG(LogTemp, Error, TEXT("EquippedAbility is of a different type. Unequipping the equipped and equipping the new"));
		UnequipAbility();
		EquipAbility(Index);
		return;
	}
	if (!EquippedAbility)
	{
		UE_LOG(LogTemp, Error, TEXT("No equipped ability. Equipping the new one."));
		EquipAbility(Index);
		return;
	}
}

void UWOGAbilitiesComponent::Server_UnequipAbility_Implementation()
{
	UnequipAbility();
}

void UWOGAbilitiesComponent::UseAbilityActionPressed()
{
	if (!EquippedAbility) return;
	bool bCanUseAbility = !EquippedAbility->GetIsInCooldown();
	if (!bCanUseAbility)
	{
		UE_LOG(LogTemp, Error, TEXT("Can't use ability yet"));
		return;
	}

	switch (EquippedAbilityType)
	{
	case EAbilityType::EAT_Projectile:
		EquippedAbility->Server_Use();
		break;
	case EAbilityType::EAT_AOE:
		break;
	case EAbilityType::EAT_Instant:
		break;
	case EAbilityType::EAT_Buff:
		break;
	default:
		break;
	}
}
