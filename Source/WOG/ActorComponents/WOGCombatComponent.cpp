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
	if (OwnerCharacter->IsLocallyControlled())
	{
		Server_CreateDefaultWeapon();
	}
}

void UWOGCombatComponent::Server_CreateDefaultWeapon_Implementation()
{
	CreateDefaultWeapon();
}

void UWOGCombatComponent::CreateDefaultWeapon()
{
	OwnerCharacter = OwnerCharacter == nullptr ? Cast<ABasePlayerCharacter>(GetOwner()) : OwnerCharacter;
	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, FString("SpawnWeapon"));
	if (!OwnerCharacter || !DefaultWeaponClass) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = OwnerCharacter;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	MainWeapon = GetWorld()->SpawnActor<AWOGBaseWeapon>(DefaultWeaponClass, OwnerCharacter->GetActorTransform(), SpawnParams);
	
	if (MainWeapon)
	{
		MainWeapon->SetOwner(OwnerCharacter);
		MainWeapon->OwnerCharacter = OwnerCharacter;
		MainWeapon->AttachToBack();
	}

}

void UWOGCombatComponent::OnRep_MainWeapon()
{
	if (MainWeapon)
	{
		MainWeapon->AttachToBack();
	}
}

void UWOGCombatComponent::OnRep_SecWeapon()
{
}

void UWOGCombatComponent::EquipWeapon()
{
	if (MainWeapon)
	{
		MainWeapon->Server_Equip();
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Orange, FString("MainWeapon->Equip() called"));
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString("Main Weapon invalid"));
	}
}

void UWOGCombatComponent::SwapWeapons()
{
}

void UWOGCombatComponent::UnequipWeapon()
{
}

void UWOGCombatComponent::DropWeapon()
{
}

void UWOGCombatComponent::AttackLight()
{
}

void UWOGCombatComponent::AttackHeavy()
{
}

void UWOGCombatComponent::Block()
{
}

void UWOGCombatComponent::StopBlocking()
{
}
