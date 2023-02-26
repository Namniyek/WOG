// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGBaseWeapon.h"
#include "Net/UnrealNetwork.h"
#include "WOG/PlayerCharacter/BasePlayerCharacter.h"
#include "WOG/ActorComponents/WOGCombatComponent.h"

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
	DOREPLIFETIME(AWOGBaseWeapon, WeaponState);
	DOREPLIFETIME(AWOGBaseWeapon, ComboStreak);
	DOREPLIFETIME(AWOGBaseWeapon, bIsInCombo);
}

void AWOGBaseWeapon::Tick(float DeltaSeconds)
{
}

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

	WeaponState = EWeaponState::EWS_Stored;
	ComboStreak = 0;
	bIsInCombo = false;
}

void AWOGBaseWeapon::Server_SetWeaponState_Implementation(EWeaponState NewWeaponState)
{
	SetWeaponState(NewWeaponState);

	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		AttachToHands();
		break;
	case EWeaponState::EWS_Stored:
		AttachToBack();
		break;
	case EWeaponState::EWS_BeingEquipped:
		break;
	case EWeaponState::EWS_BeingStored:
		break;
	case EWeaponState::EWS_Dropped:
		break;
	case EWeaponState::EWS_AttackLight:
		break;
	case EWeaponState::EWS_AttackHeavy:
		break;
	default:
		break;
	}

}

void AWOGBaseWeapon::OnRep_WeaponStateChanged()
{
	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		AttachToHands();
		break;
	case EWeaponState::EWS_Stored:
		AttachToBack();
		break;
	case EWeaponState::EWS_BeingEquipped:
		Equip();
		break;
	case EWeaponState::EWS_BeingStored:
		Unequip();
		break;
	case EWeaponState::EWS_Dropped:
		break;
	case EWeaponState::EWS_AttackLight:
		//AttackLight();
		break;
	case EWeaponState::EWS_AttackHeavy:
		AttackHeavy();
		break;
	default:
		break;
	}
}

void AWOGBaseWeapon::Server_Equip_Implementation()
{
	SetWeaponState(EWeaponState::EWS_BeingEquipped);
	Equip();
}

void AWOGBaseWeapon::Equip()
{	
	if (!OwnerCharacter) return;

	UAnimInstance* CharacterAnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
	if (CharacterAnimInstance && EquipMontage)
	{
		CharacterAnimInstance->Montage_Play(EquipMontage, 2.f);
	}
}

void AWOGBaseWeapon::AttachToHands()
{
	if (!OwnerCharacter) return;

	if (OwnerCharacter->GetCombatComponent())
	{
		OwnerCharacter->GetCombatComponent()->SetEquippedWeapon(this);
	}

	FAttachmentTransformRules AttachmentRules = FAttachmentTransformRules(
		EAttachmentRule::SnapToTarget,
		EAttachmentRule::SnapToTarget,
		EAttachmentRule::KeepWorld,
		false
	);

	MeshMain->AttachToComponent(OwnerCharacter->GetMesh(), AttachmentRules, MeshMainSocket);
	MeshSecondary->AttachToComponent(OwnerCharacter->GetMesh(), AttachmentRules, MeshSecondarySocket);

	if (HasAuthority())
	{
		SetWeaponState(EWeaponState::EWS_Equipped);
	}
}

void AWOGBaseWeapon::Server_Unequip_Implementation()
{
	SetWeaponState(EWeaponState::EWS_BeingStored);
	Unequip();
}

void AWOGBaseWeapon::Server_Swap_Implementation()
{
	SetWeaponState(EWeaponState::EWS_Stored);
	AttachToBack();
}

void AWOGBaseWeapon::Unequip()
{
	if (!OwnerCharacter) return;

	UAnimInstance* CharacterAnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
	if (CharacterAnimInstance && UnequipMontage)
	{
		CharacterAnimInstance->Montage_Play(UnequipMontage, 2.f);
	}
}

void AWOGBaseWeapon::AttachToBack()
{
	if (!OwnerCharacter) return;

	if (OwnerCharacter->GetCombatComponent())
	{
		OwnerCharacter->GetCombatComponent()->SetEquippedWeapon(nullptr);
	}

	FAttachmentTransformRules AttachmentRules = FAttachmentTransformRules(
		EAttachmentRule::SnapToTarget,
		EAttachmentRule::SnapToTarget,
		EAttachmentRule::KeepWorld,
		false
	);

	MeshMain->AttachToComponent(OwnerCharacter->GetMesh(), AttachmentRules, BackMainSocket);
	MeshSecondary->AttachToComponent(OwnerCharacter->GetMesh(), AttachmentRules, BackSecondarySocket);

	if (HasAuthority())
	{
		SetWeaponState(EWeaponState::EWS_Stored);
	}
}

void AWOGBaseWeapon::FinishAttacking()
{
	Server_SetWeaponState(EWeaponState::EWS_Equipped);
	if (OwnerCharacter)
	{
		OwnerCharacter->Server_SetCharacterState(ECharacterState::ECS_Unnoccupied);
	}
}

void AWOGBaseWeapon::Server_AttackLight_Implementation()
{
	if (ComboStreak < MaxComboStreak)
	{
		AttackLight();
		SetWeaponState(EWeaponState::EWS_AttackLight);
		Multicast_AttackLight();
		bIsInCombo = false;
	}
	if (ComboStreak == MaxComboStreak)
	{
		AttackHeavy();
		SetWeaponState(EWeaponState::EWS_AttackHeavy);
	}
}

void AWOGBaseWeapon::Multicast_AttackLight_Implementation()
{
	if (!HasAuthority())
	{
		AttackLight();
	}
}

void AWOGBaseWeapon::Server_AttackHeavy_Implementation()
{
	AttackHeavy();
	SetWeaponState(EWeaponState::EWS_AttackHeavy);
}

void AWOGBaseWeapon::AttackLight()
{
	if (!OwnerCharacter) return;

	UAnimInstance* CharacterAnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
	if (CharacterAnimInstance && AttackLightMontage)
	{
		FString SectionName = FString::FromInt(GetComboStreak());
		CharacterAnimInstance->Montage_Play(AttackLightMontage, 1.f);
		CharacterAnimInstance->Montage_JumpToSection(FName(*SectionName));
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Purple, SectionName);
	}
}

void AWOGBaseWeapon::AttackHeavy()
{
	if (!OwnerCharacter) return;

	UAnimInstance* CharacterAnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
	if (CharacterAnimInstance && AttackHeavyMontage)
	{
		CharacterAnimInstance->Montage_Play(AttackHeavyMontage, 1.f);
	}
	ComboStreak = 0;
	bIsInCombo = false;
}

void AWOGBaseWeapon::Drop()
{
}

void AWOGBaseWeapon::HandleHit()
{
}

void AWOGBaseWeapon::IncreaseCombo()
{
	if (HasAuthority())
	{
		++ComboStreak;
		bIsInCombo = true;
	}
}

void AWOGBaseWeapon::ResetCombo()
{
	if (HasAuthority())
	{
		ComboStreak = 0;
		bIsInCombo = false;

	}
}
