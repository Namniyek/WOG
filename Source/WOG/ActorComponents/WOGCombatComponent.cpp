// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGCombatComponent.h"
#include "Net/UnrealNetwork.h"
#include "WOG/PlayerCharacter/BasePlayerCharacter.h"
#include "WOG/Weapons/WOGBaseWeapon.h"
#include "DidItHitActorComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

UWOGCombatComponent::UWOGCombatComponent()
{
	SetIsReplicated(true);
}

void UWOGCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UWOGCombatComponent, MainWeapon);
	DOREPLIFETIME(UWOGCombatComponent, SecondaryWeapon);
	DOREPLIFETIME(UWOGCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UWOGCombatComponent, EquippedWeaponType);
}

void UWOGCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	OwnerPlayerCharacter = OwnerPlayerCharacter == nullptr ? Cast<ABasePlayerCharacter>(GetOwner()) : OwnerPlayerCharacter;
	/*if (OwnerPlayerCharacter->IsLocallyControlled() && DefaultWeaponClass)
	{
		Server_CreateMainWeapon(DefaultWeaponClass);
	}*/
}

void UWOGCombatComponent::Server_CreateMainWeapon_Implementation(TSubclassOf<AWOGBaseWeapon> WeaponToCreate)
{
	CreateMainWeapon(WeaponToCreate);
}

void UWOGCombatComponent::CreateMainWeapon(TSubclassOf<AWOGBaseWeapon> WeaponToCreate)
{
	OwnerPlayerCharacter = OwnerPlayerCharacter == nullptr ? Cast<ABasePlayerCharacter>(GetOwner()) : OwnerPlayerCharacter;
	if (!OwnerPlayerCharacter) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = OwnerPlayerCharacter;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	MainWeapon = GetWorld()->SpawnActor<AWOGBaseWeapon>(WeaponToCreate, OwnerPlayerCharacter->GetActorTransform(), SpawnParams);
	
	if (MainWeapon)
	{
		MainWeapon->SetOwner(OwnerPlayerCharacter);
		MainWeapon->OwnerCharacter = OwnerPlayerCharacter;
		MainWeapon->Server_SetWeaponState(EWeaponState::EWS_Stored);
		MainWeapon->InitTraceComponent();

		if (MainWeapon->GetTraceComponent() && SecondaryWeapon)
		{
			MainWeapon->GetTraceComponent()->MyActorsToIgnore.AddUnique(SecondaryWeapon);
		}
	}

}

void UWOGCombatComponent::OnRep_MainWeapon()
{
	MainWeapon->AttachToBack();
}

void UWOGCombatComponent::Server_CreateSecondaryWeapon_Implementation(TSubclassOf<AWOGBaseWeapon> WeaponToCreate)
{
	CreateSecondaryWeapon(WeaponToCreate);
}

void UWOGCombatComponent::CreateSecondaryWeapon(TSubclassOf<AWOGBaseWeapon> WeaponToCreate)
{
	OwnerPlayerCharacter = OwnerPlayerCharacter == nullptr ? Cast<ABasePlayerCharacter>(GetOwner()) : OwnerPlayerCharacter;
	if (!OwnerPlayerCharacter) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = OwnerPlayerCharacter;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	SecondaryWeapon = GetWorld()->SpawnActor<AWOGBaseWeapon>(WeaponToCreate, OwnerPlayerCharacter->GetActorTransform(), SpawnParams);

	if (SecondaryWeapon)
	{
		SecondaryWeapon->SetOwner(OwnerPlayerCharacter);
		SecondaryWeapon->OwnerCharacter = OwnerPlayerCharacter;
		SecondaryWeapon->Server_SetWeaponState(EWeaponState::EWS_Stored);
		SecondaryWeapon->InitTraceComponent();

		if (SecondaryWeapon->GetTraceComponent() && MainWeapon)
		{
			SecondaryWeapon->GetTraceComponent()->MyActorsToIgnore.AddUnique(MainWeapon);
		}
	}

}

void UWOGCombatComponent::OnRep_SecWeapon()
{
	SecondaryWeapon->AttachToBack();
}

void UWOGCombatComponent::SetEquippedWeapon(AWOGBaseWeapon* NewEquippedWeapon)
{
	EquippedWeapon = NewEquippedWeapon;
	if (EquippedWeapon)
	{
		EquippedWeaponType = EquippedWeapon->GetWeaponType();
	}
	if (EquippedWeapon == nullptr)
	{
		EquippedWeaponType = EWeaponType::EWT_Relax;
	}
}

void UWOGCombatComponent::EquipMainWeapon()
{

	if (MainWeapon)
	{
		MainWeapon->Server_Equip();
	}
}

void UWOGCombatComponent::EquipSecondaryWeapon()
{

	if (SecondaryWeapon)
	{
		SecondaryWeapon->Server_Equip();
	}
}

void UWOGCombatComponent::UnequipMainWeapon()
{

	if (MainWeapon)
	{
		MainWeapon->Server_Unequip();
	}
}

void UWOGCombatComponent::UnequipSecondaryWeapon()
{

	if (SecondaryWeapon)
	{
		SecondaryWeapon->Server_Unequip();
	}
}

void UWOGCombatComponent::SwapWeapons()
{

	if (EquippedWeapon == MainWeapon)
	{
		MainWeapon->Server_Swap();
		SecondaryWeapon->Server_Equip();
	}
	else if (EquippedWeapon == SecondaryWeapon)
	{
		SecondaryWeapon->Server_Swap();
		MainWeapon->Server_Equip();
	}
	if (OwnerPlayerCharacter)
	{
		OwnerPlayerCharacter->Server_SetCharacterState(ECharacterState::ECS_Unnoccupied);
	}
}

void UWOGCombatComponent::DropWeapons()
{
}

void UWOGCombatComponent::AttackLight()
{
	OwnerPlayerCharacter = OwnerPlayerCharacter == nullptr ? Cast<ABasePlayerCharacter>(GetOwner()) : OwnerPlayerCharacter;
	if (!EquippedWeapon || !OwnerPlayerCharacter) return;

	if (OwnerPlayerCharacter->CharacterState != ECharacterState::ECS_Attacking || EquippedWeapon->GetIsInCombo())
	{
		EquippedWeapon->Server_AttackLight();
		OwnerPlayerCharacter->Server_SetCharacterState(ECharacterState::ECS_Attacking);
	}
}

void UWOGCombatComponent::AttackHeavy()
{
	OwnerPlayerCharacter = OwnerPlayerCharacter == nullptr ? Cast<ABasePlayerCharacter>(GetOwner()) : OwnerPlayerCharacter;
	if (!EquippedWeapon || !OwnerPlayerCharacter) return;
	OwnerPlayerCharacter->Server_SetCharacterState(ECharacterState::ECS_Attacking);

	EquippedWeapon->Server_AttackHeavy();
}

void UWOGCombatComponent::Block()
{
	OwnerPlayerCharacter = OwnerPlayerCharacter == nullptr ? Cast<ABasePlayerCharacter>(GetOwner()) : OwnerPlayerCharacter;
	if (!OwnerPlayerCharacter) return;
	if (OwnerPlayerCharacter->GetCharacterState() == ECharacterState::ECS_Unnoccupied)
	{
		OwnerPlayerCharacter->Server_SetCharacterState(ECharacterState::ECS_Blocking);
	}

	if (!EquippedWeapon)
	{
		return;
	}
	EquippedWeapon->Server_Block();
}

void UWOGCombatComponent::StopBlocking()
{
	OwnerPlayerCharacter = OwnerPlayerCharacter == nullptr ? Cast<ABasePlayerCharacter>(GetOwner()) : OwnerPlayerCharacter;
	if (!OwnerPlayerCharacter) return;

	if (OwnerPlayerCharacter->GetCharacterState() == ECharacterState::ECS_Blocking)
	{
		OwnerPlayerCharacter->Server_SetCharacterState(ECharacterState::ECS_Unnoccupied);
	}

	if (!EquippedWeapon)
	{
		return;
	}
	EquippedWeapon->Server_StopBlocking();

}
