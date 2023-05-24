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
		EquippedWeaponType = EquippedWeapon->GetWeaponData().WeaponType;
	}
	if (EquippedWeapon == nullptr)
	{
		EquippedWeaponType = EWeaponType::EWT_Relax;
	}
}
