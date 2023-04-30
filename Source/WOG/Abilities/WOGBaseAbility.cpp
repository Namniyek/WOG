// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGBaseAbility.h"
#include "Net/UnrealNetwork.h"

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
	
}

void AWOGBaseAbility::Equip()
{
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
	if (!HasAuthority())
	{
		CosmeticUse();
	}
}

void AWOGBaseAbility::Server_Use_Implementation()
{
	Use();
	CosmeticUse();
	Multicast_CosmeticUse();
}

void AWOGBaseAbility::Server_Unequip_Implementation()
{
	Unequip();
}

void AWOGBaseAbility::Server_Equip_Implementation()
{
	Equip();
}

