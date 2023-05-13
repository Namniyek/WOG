// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGBaseAbility.h"
#include "Net/UnrealNetwork.h"
#include "WOG/PlayerCharacter/BasePlayerCharacter.h"
#include "WOG/Data/WOGAbilityDataAsset.h"

AWOGBaseAbility::AWOGBaseAbility()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);
	bIsInCooldown = false;
}

void AWOGBaseAbility::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWOGBaseAbility, bIsInCooldown);
}

void AWOGBaseAbility::BeginPlay()
{
	Super::BeginPlay();
	
	Init();
}

void AWOGBaseAbility::Init()
{
	if (!AbilityDataAsset)
	{
		UE_LOG(LogTemp, Error, TEXT("No Valid Data Asset"));
		return;
	}

	AbilityType = AbilityDataAsset->AbilityType;
	CooldownTime = AbilityDataAsset->CooldownTime;
	EquipMontage = AbilityDataAsset->EquipMontage;
	UseMontage = AbilityDataAsset->UseMontage;
	IdleSound = AbilityDataAsset->IdleSound;
	UseSound = AbilityDataAsset->UseSound;
	IdleParticleSystem = AbilityDataAsset->IdleParticleSystem;
	UseParticleSystem = AbilityDataAsset->UseParticleSystem;
}

void AWOGBaseAbility::Equip()
{
	//TO BE OVERRIDEN
}

void AWOGBaseAbility::Unequip()
{
	Destroy();
}

void AWOGBaseAbility::Use()
{
	bIsInCooldown = true;
	GetWorldTimerManager().SetTimer(CooldownTimer, this, &ThisClass::ResetCooldown, CooldownTime);
	UE_LOG(LogTemp, Warning, TEXT("Cooldown started"));
}



void AWOGBaseAbility::CosmeticEquip()
{

}

void AWOGBaseAbility::CosmeticUnequip()
{

}

void AWOGBaseAbility::Multicast_CosmeticEquip_Implementation()
{
	if (HasAuthority() || (OwnerCharacter && OwnerCharacter->IsLocallyControlled())) return;
	CosmeticEquip();
}

void AWOGBaseAbility::Multicast_CosmeticUnequip_Implementation()
{
	if (HasAuthority()||OwnerCharacter && OwnerCharacter->IsLocallyControlled()) return;
	CosmeticUnequip();
}

void AWOGBaseAbility::CosmeticUse()
{

}

void AWOGBaseAbility::ResetCooldown()
{
	bIsInCooldown = false;
	UE_LOG(LogTemp, Warning, TEXT("Cooldown ended"));
}

void AWOGBaseAbility::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWOGBaseAbility::Multicast_CosmeticUse_Implementation()
{
	if (HasAuthority() || OwnerCharacter && OwnerCharacter->IsLocallyControlled()) return;
	CosmeticUse();

}

void AWOGBaseAbility::Server_Use_Implementation()
{
	Use();
	CosmeticUse();
	Multicast_CosmeticUse();
}

void AWOGBaseAbility::Server_Unequip_Implementation()
{
	Multicast_CosmeticUnequip();
	Unequip();
}

void AWOGBaseAbility::Server_Equip_Implementation()
{
	Equip();
	Multicast_CosmeticEquip();
}

