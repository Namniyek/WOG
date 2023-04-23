// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGBaseWeapon.h"
#include "Net/UnrealNetwork.h"
#include "WOG/PlayerCharacter/BasePlayerCharacter.h"
#include "WOG/Enemies/WOGBaseEnemy.h"
#include "WOG/ActorComponents/WOGCombatComponent.h"
#include "DidItHitActorComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "WOG/Interfaces/BuildingInterface.h"
#include "WOG/Interfaces/AttributesInterface.h"

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

	TraceComponent = CreateDefaultSubobject <UDidItHitActorComponent>(TEXT("TraceComponent"));
}

void AWOGBaseWeapon::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	InitWeapon();
}

void AWOGBaseWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWOGBaseWeapon, OwnerCharacter);
	DOREPLIFETIME(AWOGBaseWeapon, WeaponState);
	DOREPLIFETIME(AWOGBaseWeapon, ComboStreak);
	DOREPLIFETIME(AWOGBaseWeapon, bIsInCombo);
	DOREPLIFETIME(AWOGBaseWeapon, bIsBlocking);
	DOREPLIFETIME(AWOGBaseWeapon, bCanParry);
}

void AWOGBaseWeapon::Tick(float DeltaSeconds)
{
}

void AWOGBaseWeapon::BeginPlay()
{
	Super::BeginPlay();
	//InitWeapon();

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

		AttackMontage = WeaponDataRow->AttackMontage;
		DodgeMontage = WeaponDataRow->DodgeMontage;
		BlockMontage = WeaponDataRow->BlockMontage;
		EquipMontage = WeaponDataRow->EquipMontage;
		HurtMontage = WeaponDataRow->HurtMontage;

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

	AttachToBack();

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

void AWOGBaseWeapon::SetCanNotParry()
{
	bCanParry = false;
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
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, FString("OnRep Called()"));
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
		break;
	case EWeaponState::EWS_AttackHeavy:
		AttackHeavy();
		break;
	case EWeaponState::EWS_Blocking:
		//Block();
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
		CharacterAnimInstance->Montage_Play(EquipMontage, 1.f);
		CharacterAnimInstance->Montage_JumpToSection(FName("Equip"), EquipMontage);
	}
}

void AWOGBaseWeapon::AttachToHands()
{
	if (!OwnerCharacter) return;

	if (HasAuthority() && OwnerCharacter->GetCombatComponent())
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
	if (CharacterAnimInstance && EquipMontage)
	{
		CharacterAnimInstance->Montage_Play(EquipMontage, 1.f);
		CharacterAnimInstance->Montage_JumpToSection(FName("Unequip"), EquipMontage);
	}
}

void AWOGBaseWeapon::InitTraceComponent()
{
	if (TraceComponent)
	{
		TraceComponent->bHitOtherSocketsAtDifferentTime = true;
		TraceComponent->bHitOtherSocketsAtSameTime = true;
		TraceComponent->bHitSameSocketAtDifferentTimes = true;

		TraceComponent->SetupVariables(MeshMain, OwnerCharacter);
		if (!TraceComponent->MyActorsToIgnore.Contains(OwnerCharacter))
		{
			TraceComponent->MyActorsToIgnore.Add(OwnerCharacter);
		}
		TraceComponent->ToggleTraceCheck(false);
		TraceComponent->MyWorldContextObject = this;
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Cyan, FString("SetupVariables() called"));

		if (HasAuthority())
		{
			TraceComponent->OnItemAdded.AddDynamic(this, &ThisClass::HitDetected);
		}
	}
}

void AWOGBaseWeapon::AttachToBack()
{
	if (!OwnerCharacter) return;

	if (HasAuthority() && OwnerCharacter->GetCombatComponent() && OwnerCharacter->GetCombatComponent()->GetEquippedWeapon())
	{
		if (OwnerCharacter->GetCombatComponent()->GetEquippedWeapon()->GetWeaponState() == EWeaponState::EWS_BeingStored
			|| OwnerCharacter->GetCombatComponent()->GetEquippedWeapon()->GetWeaponState() == EWeaponState::EWS_Stored)
		{
			OwnerCharacter->GetCombatComponent()->SetEquippedWeapon(nullptr);
		}
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
	if (TraceComponent)
	{
		TraceComponent->ToggleTraceCheck(false);
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
	if (CharacterAnimInstance && AttackMontage)
	{
		FString SectionName = FString::FromInt(GetComboStreak());
		CharacterAnimInstance->Montage_Play(AttackMontage, 1.f);
		CharacterAnimInstance->Montage_JumpToSection(FName(*SectionName));
	}
	if (TraceComponent)
	{
		TraceComponent->ToggleTraceCheck(false);
		HitActorsToIgnore.Empty();
		if (OwnerCharacter->GetCombatComponent()->GetSecondaryWeapon())
		{
			HitActorsToIgnore.Add(OwnerCharacter->GetCombatComponent()->GetSecondaryWeapon());
		}
		if (OwnerCharacter->GetCombatComponent()->GetMainWeapon())
		{
			HitActorsToIgnore.Add(OwnerCharacter->GetCombatComponent()->GetMainWeapon());
		}
		TraceComponent->ToggleTraceCheck(true);
	}

	if (SwingSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, SwingSound, GetActorLocation());
	}
}

void AWOGBaseWeapon::AttackHeavy()
{
	if (!OwnerCharacter) return;

	UAnimInstance* CharacterAnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
	if (CharacterAnimInstance && AttackMontage)
	{
		CharacterAnimInstance->Montage_Play(AttackMontage, 1.f);
		CharacterAnimInstance->Montage_JumpToSection(FName("Heavy"), AttackMontage);
	}
	ComboStreak = 0;
	bIsInCombo = false;

	if (TraceComponent)
	{
		TraceComponent->ToggleTraceCheck(false);
		HitActorsToIgnore.Empty();
		TraceComponent->ToggleTraceCheck(true);
	}

	if (SwingSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, SwingSound, GetActorLocation());
	}
}

void AWOGBaseWeapon::Drop()
{
}

void AWOGBaseWeapon::Server_Block_Implementation()
{
	Multicast_Block();
	SetWeaponState(EWeaponState::EWS_Blocking);
	Block();
	bCanParry = true;
	GetWorldTimerManager().SetTimer(ParryTimer, this, &ThisClass::SetCanNotParry, MaxParryThreshold);
}

void AWOGBaseWeapon::Multicast_Block_Implementation()
{
	if (!HasAuthority())
	{
		Block();
	}
}

void AWOGBaseWeapon::Block()
{
	if (!OwnerCharacter) return;

	UAnimInstance* CharacterAnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
	if (CharacterAnimInstance && BlockMontage)
	{
		CharacterAnimInstance->Montage_Play(BlockMontage, 1.f);
		CharacterAnimInstance->Montage_JumpToSection(FName("Loop"));
	}
}

void AWOGBaseWeapon::Server_StopBlocking_Implementation()
{
	Multicast_StopBlocking();
	StopBlocking();
	//bIsBlocking = false;
	SetWeaponState(EWeaponState::EWS_Equipped);
}

void AWOGBaseWeapon::Multicast_StopBlocking_Implementation()
{
	if (!HasAuthority())
	{
		StopBlocking();
	}
}

void AWOGBaseWeapon::StopBlocking()
{
	if (!OwnerCharacter) return;

	UAnimInstance* CharacterAnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
	if (CharacterAnimInstance && BlockMontage)
	{
		//FString SectionName = FString::FromInt(GetComboStreak());
		CharacterAnimInstance->Montage_Play(BlockMontage, 1.f);
		CharacterAnimInstance->Montage_JumpToSection(FName("Stop"));
	}
}

void AWOGBaseWeapon::HitDetected(FHitResult Hit)
{
	if (!Hit.bBlockingHit || !Hit.GetActor()) return;

	float DamageToApply = 0.f;
	switch (WeaponState)
	{
	case EWeaponState::EWS_AttackLight:
		DamageToApply = BaseDamage + (BaseDamage * DamageMultiplier) + (BaseDamage * (ComboStreak * ComboDamageMultiplier));
		break;
	case EWeaponState::EWS_AttackHeavy:
		DamageToApply = BaseDamage + (BaseDamage * DamageMultiplier) + (BaseDamage * HeavyDamageMultiplier);
		break;
	}

	TObjectPtr<IBuildingInterface> BuildInterface = Cast<IBuildingInterface>(Hit.GetActor());
	
	if (BuildInterface)
	{
		if (HitActorsToIgnore.Contains(Hit.GetActor()))
		{
			//Build already hit
			return;
		}

		HitActorsToIgnore.AddUnique(Hit.GetActor());
		BuildInterface->Execute_DealDamage(Hit.GetActor(), DamageToApply);
		UE_LOG(LogTemp, Warning, TEXT("Build damaged with %f"), DamageToApply);
		return;

	}

	TObjectPtr<AActor> HitActor;
	if (Hit.GetActor()->GetClass()->IsChildOf<AWOGBaseCharacter>())
	{
		HitActor = Hit.GetActor();
	}
	else
	{
		if (Hit.GetActor()->GetOwner() && Hit.GetActor()->GetOwner()->GetClass()->IsChildOf<AWOGBaseCharacter>())
		{
			HitActor = Hit.GetActor()->GetOwner();
		}
	}

	if (!HitActor || HitActorsToIgnore.Contains(HitActor)) return;
	HitActorsToIgnore.AddUnique(HitActor);

	TObjectPtr<IAttributesInterface> AttributesInterface = Cast<IAttributesInterface>(HitActor);
	if (AttributesInterface)
	{
		AttributesInterface->Execute_BroadcastHit(HitActor, OwnerCharacter, Hit, DamageToApply, this);
	}
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
