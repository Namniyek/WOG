// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGBaseWeapon.h"
#include "Net/UnrealNetwork.h"
#include "WOG/PlayerCharacter/BasePlayerCharacter.h"

// Sets default values
AWOGBaseWeapon::AWOGBaseWeapon()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	MeshMain = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Main Mesh"));
	MeshMain->SetupAttachment(GetRootComponent());
	MeshMain->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	MeshSecondary = CreateDefaultSubobject <UStaticMeshComponent>(TEXT("Secondary Mesh"));
	MeshSecondary->SetupAttachment(GetRootComponent());
	MeshSecondary->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AWOGBaseWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWOGBaseWeapon, OwnerCharacter);
}

void AWOGBaseWeapon::Tick(float DeltaSeconds)
{
}

// Called when the game starts or when spawned
void AWOGBaseWeapon::BeginPlay()
{
	Super::BeginPlay();
	InitWeapon();

}

void AWOGBaseWeapon::InitWeapon()
{
	const FString WeaponTablePath{ TEXT("Engine.DataTable'/Game/Data/Weapons/DT_Weapons.DT_Weapons'") };
	UDataTable* WeaponTableObject = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *WeaponTablePath));

	if (!WeaponTableObject) return;

	TArray<FName> WeaponNamesArray = WeaponTableObject->GetRowNames();
	FWeaponDataTable* WeaponDataRow = nullptr;
	
	for (auto WeaponRowName : WeaponNamesArray)
	{
		if (WeaponRowName == WeaponName)
		{
			WeaponDataRow = WeaponTableObject->FindRow<FWeaponDataTable>(WeaponName, TEXT(""));
			break;
		}
	}
	if (WeaponDataRow)
	{
		MeshMain->SetStaticMesh(WeaponDataRow->MeshMain);
		MeshSecondary->SetStaticMesh(WeaponDataRow->MeshSecondary);
		bIsAttacker = WeaponDataRow->bIsAttacker;
		WeaponName = WeaponDataRow->WeaponName;
		WeaponType = WeaponDataRow->WeaponType;

		AttackLightMontage = WeaponDataRow->AttackLightMontage;
		AttackHeavyMontage = WeaponDataRow->AttackHeavyMontage;
		BlockMontage = WeaponDataRow->BlockMontage;
		ParryMontage = WeaponDataRow->ParryMontage;
		EquipMontage = WeaponDataRow->EquipMontage;
		UnequipMontage = WeaponDataRow->UnequipMontage;

		BaseDamage = WeaponDataRow->BaseDamage;
		HeavyDamageMultiplier = WeaponDataRow->HeavyDamageMultiplier;
		DamageMultiplier = WeaponDataRow->DamageMultiplier;
		ComboDamageMultiplier = WeaponDataRow->ComboDamageMultiplier;
		MaxComboStreak = WeaponDataRow->MaxComboStreak;
		MaxParryThreshold = WeaponDataRow->MaxParryThreshold;

		SwingSound = WeaponDataRow->SwingSound;
		HitSound = WeaponDataRow->HitSound;
		BlockSound = WeaponDataRow->BlockSound;

		MeshMainSocket = WeaponDataRow->MeshMainSocket;
		MeshSecondarySocket = WeaponDataRow->MeshSecondarySocket;
		BackMainSocket = WeaponDataRow->BackMainSocket;
		BackSecondarySocket = WeaponDataRow->BackSecondarySocket;

	}
}

void AWOGBaseWeapon::AttachToBack()
{
	if (OwnerCharacter)
	{
		FAttachmentTransformRules AttachmentRules = FAttachmentTransformRules(
			EAttachmentRule::SnapToTarget,
			EAttachmentRule::SnapToTarget,
			EAttachmentRule::KeepWorld,
			false
		);

		MeshMain->AttachToComponent(OwnerCharacter->GetMesh(), AttachmentRules, BackMainSocket);
		MeshSecondary->AttachToComponent(OwnerCharacter->GetMesh(), AttachmentRules, BackSecondarySocket);
	}
}

void AWOGBaseWeapon::Server_Equip_Implementation()
{
	Multicast_Equip();
}

void AWOGBaseWeapon::Multicast_Equip_Implementation()
{
	Equip();
}

void AWOGBaseWeapon::Equip()
{	
	if (OwnerCharacter)
	{
		FAttachmentTransformRules AttachmentRules = FAttachmentTransformRules(
			EAttachmentRule::SnapToTarget, 
			EAttachmentRule::SnapToTarget, 
			EAttachmentRule::KeepWorld, 
			false
		);

		MeshMain->AttachToComponent(OwnerCharacter->GetMesh(), AttachmentRules, MeshMainSocket);
		MeshSecondary->AttachToComponent(OwnerCharacter->GetMesh(), AttachmentRules, MeshSecondarySocket);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, FString("no Owner character"));
	}
}

void AWOGBaseWeapon::Unequip()
{
}

void AWOGBaseWeapon::Drop()
{
}

void AWOGBaseWeapon::HandleHit()
{
}


