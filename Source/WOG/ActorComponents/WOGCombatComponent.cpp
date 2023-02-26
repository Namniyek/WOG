// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGCombatComponent.h"
#include "Net/UnrealNetwork.h"
#include "WOG/PlayerCharacter/BasePlayerCharacter.h"
#include "WOG/Weapons/WOGBaseWeapon.h"

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
	OwnerCharacter = OwnerCharacter == nullptr ? Cast<ABasePlayerCharacter>(GetOwner()) : OwnerCharacter;
	if (OwnerCharacter->IsLocallyControlled() && DefaultWeaponClass)
	{
		Server_CreateMainWeapon(DefaultWeaponClass);
	}
}

void UWOGCombatComponent::Server_CreateMainWeapon_Implementation(TSubclassOf<AWOGBaseWeapon> WeaponToCreate)
{
	CreateMainWeapon(WeaponToCreate);
}

void UWOGCombatComponent::CreateMainWeapon(TSubclassOf<AWOGBaseWeapon> WeaponToCreate)
{
	OwnerCharacter = OwnerCharacter == nullptr ? Cast<ABasePlayerCharacter>(GetOwner()) : OwnerCharacter;
	if (!OwnerCharacter) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = OwnerCharacter;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	MainWeapon = GetWorld()->SpawnActor<AWOGBaseWeapon>(WeaponToCreate, OwnerCharacter->GetActorTransform(), SpawnParams);
	
	if (MainWeapon)
	{
		MainWeapon->SetOwner(OwnerCharacter);
		MainWeapon->OwnerCharacter = OwnerCharacter;
		MainWeapon->Server_SetWeaponState(EWeaponState::EWS_Stored);
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
	OwnerCharacter = OwnerCharacter == nullptr ? Cast<ABasePlayerCharacter>(GetOwner()) : OwnerCharacter;
	if (!OwnerCharacter) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = OwnerCharacter;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	SecondaryWeapon = GetWorld()->SpawnActor<AWOGBaseWeapon>(WeaponToCreate, OwnerCharacter->GetActorTransform(), SpawnParams);

	if (SecondaryWeapon)
	{
		SecondaryWeapon->SetOwner(OwnerCharacter);
		SecondaryWeapon->OwnerCharacter = OwnerCharacter;
		SecondaryWeapon->Server_SetWeaponState(EWeaponState::EWS_Stored);
	}

}

void UWOGCombatComponent::OnRep_SecWeapon()
{
	SecondaryWeapon->AttachToBack();
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
	if (OwnerCharacter)
	{
		OwnerCharacter->Server_SetCharacterState(ECharacterState::ECS_Unnoccupied);
	}
}

void UWOGCombatComponent::DropWeapons()
{
}

void UWOGCombatComponent::AttackLight()
{
	OwnerCharacter = OwnerCharacter == nullptr ? Cast<ABasePlayerCharacter>(GetOwner()) : OwnerCharacter;
	if (!EquippedWeapon || !OwnerCharacter) return;

	if (OwnerCharacter->CharacterState != ECharacterState::ECS_Attacking || EquippedWeapon->GetIsInCombo())
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Black, FString("Light Attack"));
		EquippedWeapon->Server_AttackLight();
		OwnerCharacter->Server_SetCharacterState(ECharacterState::ECS_Attacking);
	}
}

void UWOGCombatComponent::AttackHeavy()
{
	OwnerCharacter = OwnerCharacter == nullptr ? Cast<ABasePlayerCharacter>(GetOwner()) : OwnerCharacter;
	if (!EquippedWeapon || !OwnerCharacter) return;
	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Black, FString("Heavy Attack"));
	OwnerCharacter->Server_SetCharacterState(ECharacterState::ECS_Attacking);

	EquippedWeapon->Server_AttackHeavy();
}

void UWOGCombatComponent::Block()
{
}

void UWOGCombatComponent::StopBlocking()
{
}
