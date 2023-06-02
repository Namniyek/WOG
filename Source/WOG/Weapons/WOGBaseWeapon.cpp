// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGBaseWeapon.h"
#include "WOG.h"
#include "Net/UnrealNetwork.h"
#include "WOG/PlayerCharacter/BasePlayerCharacter.h"
#include "WOG/Enemies/WOGBaseEnemy.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "WOG/Interfaces/BuildingInterface.h"
#include "WOG/Interfaces/AttributesInterface.h"
#include "Components/SphereComponent.h"
#include "Data/AGRLibrary.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Types/WOGGameplayTags.h"
#include "GameplayTags.h"

// Sets default values
AWOGBaseWeapon::AWOGBaseWeapon()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);
	bNetLoadOnClient = false;

	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapSphere"));
	SetRootComponent(SphereComponent);
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SphereComponent->InitSphereRadius(96.f);
	SphereComponent->SetGenerateOverlapEvents(true);

	MeshMain = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Main Mesh"));
	MeshMain->SetupAttachment(GetRootComponent());
	MeshMain->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshMain->SetIsReplicated(true);

	MeshSecondary = CreateDefaultSubobject <UStaticMeshComponent>(TEXT("Secondary Mesh"));
	MeshSecondary->SetupAttachment(GetRootComponent());
	MeshSecondary->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshSecondary->SetIsReplicated(true);

	ItemComponent = CreateDefaultSubobject <UAGR_ItemComponent>(TEXT("ItemComponent"));

}

void AWOGBaseWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWOGBaseWeapon, OwnerCharacter);
}

void AWOGBaseWeapon::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	InitWeaponData();
}

void AWOGBaseWeapon::InitWeaponData()
{
	OwnerCharacter = GetOwner() ? Cast<ABasePlayerCharacter>(GetOwner()) : nullptr;

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
		WeaponData.bIsAttacker = WeaponDataRow->bIsAttacker;
		WeaponData.WeaponName = WeaponDataRow->WeaponName;
		WeaponData.WeaponType = WeaponDataRow->WeaponType;
		WeaponData.WeaponTag = WeaponDataRow->WeaponTag;
		WeaponData.WeaponPoseTag = WeaponDataRow->WeaponPoseTag;

		WeaponData.AttackMontage = WeaponDataRow->AttackMontage;
		WeaponData.DodgeMontage = WeaponDataRow->DodgeMontage;
		WeaponData.BlockMontage = WeaponDataRow->BlockMontage;
		WeaponData.EquipMontage = WeaponDataRow->EquipMontage;
		WeaponData.HurtMontage = WeaponDataRow->HurtMontage;

		WeaponData.BaseDamage = WeaponDataRow->BaseDamage;
		WeaponData.HeavyDamageMultiplier = WeaponDataRow->HeavyDamageMultiplier;
		WeaponData.DamageMultiplier = WeaponDataRow->DamageMultiplier;
		WeaponData.ComboDamageMultiplier = WeaponDataRow->ComboDamageMultiplier;
		WeaponData.MaxComboStreak = WeaponDataRow->MaxComboStreak;
		WeaponData.MaxParryThreshold = WeaponDataRow->MaxParryThreshold;

		WeaponData.SwingSound = WeaponDataRow->SwingSound;
		WeaponData.HitSound = WeaponDataRow->HitSound;
		WeaponData.BlockSound = WeaponDataRow->BlockSound;

		WeaponData.MeshMainSocket = WeaponDataRow->MeshMainSocket;
		WeaponData.MeshSecondarySocket = WeaponDataRow->MeshSecondarySocket;
		WeaponData.BackMainSocket = WeaponDataRow->BackMainSocket;
		WeaponData.BackSecondarySocket = WeaponDataRow->BackSecondarySocket;

		WeaponData.WeaponDamageEffect = WeaponDataRow->WeaponDamageEffect;
	}

	//WeaponState = EWeaponState::EWS_Stored;
	ComboStreak = 0;
	bAttackWindowOpen = false;

	AttachToBack();

}

void AWOGBaseWeapon::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (SphereComponent && HasAuthority())
	{
		SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnWeaponOverlap);
	}
	if (ItemComponent && HasAuthority())
	{
		ItemComponent->ItemName = WeaponName;
		ItemComponent->ItemTagSlotType = WeaponData.WeaponTag;

		ItemComponent->OnPickup.AddDynamic(this, &ThisClass::OnWeaponPickedUp);
		ItemComponent->OnEquip.AddDynamic(this, &ThisClass::OnWeaponEquip);
		ItemComponent->OnUnEquip.AddDynamic(this, &ThisClass::OnWeaponUnequip);
	}
}

void AWOGBaseWeapon::BeginPlay()
{
	Super::BeginPlay();
	OwnerCharacter = OwnerCharacter!=nullptr ? OwnerCharacter : GetOwner() ? Cast<ABasePlayerCharacter>(GetOwner()) : nullptr;
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("NO OWNER CHARACTER AT BEGINPLAY"));
	}

}

void AWOGBaseWeapon::OnWeaponOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority() || !ItemComponent) return;

	UAGR_EquipmentManager* Equipment = UAGRLibrary::GetEquipment(OtherActor);
	UAGR_InventoryManager* Inventory = UAGRLibrary::GetInventory(OtherActor);

	if (!Equipment || !Inventory)
	{
		UE_LOG(LogTemp, Error, TEXT("Equipment or Inventory not valid"));
		return;
	}
	
	/*
	** 
	**TO - DO Loop through inventory to check for duplicate items
	**
	*/

	int32 MaxAmountWeapons = WeaponData.bIsAttacker ? 1 : 2;
	if (Equipment->WeaponShortcutReferences.Num() < MaxAmountWeapons)
	{
		FShortcutItemReference WeaponRef;
		int32 KeyInt = Equipment->WeaponShortcutReferences.Num() + 1;
		WeaponRef.Key = *FString::FromInt(KeyInt);
		WeaponRef.ItemShortcut = this;

		if (Equipment->SaveWeaponShortcutReference(WeaponRef))
		{
			ABasePlayerCharacter* NewOwnerCharacter = Cast<ABasePlayerCharacter>(OtherActor);
			if (NewOwnerCharacter)
			{
				SetOwnerCharacter(NewOwnerCharacter);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("OWNER CHARACTER NOT VALID"));
			}
			SphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			SphereComponent->SetGenerateOverlapEvents(false);
			ItemComponent->PickUpItem(Inventory);
			if (OwnerCharacter)
			{
				OwnerCharacter->Server_EquipWeapon(WeaponRef.Key, this);
			}
			return;
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Too many weapons in the inventory"));
	}
}

void AWOGBaseWeapon::OnWeaponPickedUp(UAGR_InventoryManager* Inventory)
{
	/*
	** BOUND AND KEPT HERE JUST IN CASE
	*/
}

void AWOGBaseWeapon::OnWeaponEquip(AActor* User, FName SlotName)
{
	Multicast_OnWeaponEquip(User, SlotName);
	UE_LOG(LogTemp, Display, TEXT("WeaponEquipped"));

	if (!User) return;
	UAGRAnimMasterComponent* AnimMaster = UAGRLibrary::GetAnimationMaster(User);
	if (!AnimMaster) return;

	if (SlotName == NAME_WeaponSlot_Primary)
	{
		AnimMaster->SetupBasePose(WeaponData.WeaponPoseTag);
	}
	else
	{
		AnimMaster->SetupBasePose(TAG_Pose_Relax);
	}
}

void AWOGBaseWeapon::Multicast_OnWeaponEquip_Implementation(AActor* User, FName SlotName)
{
	SetTraceMeshes(SlotName, User);

	if (SlotName == NAME_WeaponSlot_Primary)
	{
		AttachToHands();
	}
	else
	{
		AttachToBack();
	}
}

void AWOGBaseWeapon::OnWeaponUnequip(AActor* User, FName SlotName)
{
	/*
	** BOUND AND KEPT HERE JUST IN CASE
	*/
}

void AWOGBaseWeapon::SetCanNotParry()
{
	bCanParry = false;
}

void AWOGBaseWeapon::AttachToHands()
{
	if (!OwnerCharacter) return;
	AttachToActor(OwnerCharacter, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

	MeshMain->AttachToComponent(OwnerCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponData.MeshMainSocket);
	MeshSecondary->AttachToComponent(OwnerCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponData.MeshSecondarySocket);
}

void AWOGBaseWeapon::AttachToBack()
{
	if (!OwnerCharacter) return;
	AttachToActor(OwnerCharacter, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

	MeshMain->AttachToComponent(OwnerCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponData.BackMainSocket);
	MeshSecondary->AttachToComponent(OwnerCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponData.BackSecondarySocket);
}

void AWOGBaseWeapon::AttackLight()
{
	if (!OwnerCharacter) return;

	bAttackWindowOpen = false;
}

void AWOGBaseWeapon::AttackHeavy()
{
	if (!OwnerCharacter) return;

	bAttackWindowOpen = false;

	ComboStreak = 0;
}

void AWOGBaseWeapon::Block()
{
	if (!OwnerCharacter || !HasAuthority()) return;

	bCanParry = true;
	GetWorldTimerManager().SetTimer(ParryTimer, this, &ThisClass::SetCanNotParry, WeaponData.MaxParryThreshold);
}

void AWOGBaseWeapon::StopBlocking()
{
	// Kept here just in case...
}

void AWOGBaseWeapon::SetTraceMeshes(const FName& Slot, AActor* OwnerActor)
{
	if (!OwnerActor) return;

	UAGR_CombatManager* CombatManager = UAGRLibrary::GetCombatManager(OwnerActor);
	if (!CombatManager) return;

	if (Slot == NAME_WeaponSlot_Primary)
	{
		if (MeshMain)
		{
			CombatManager->AddTraceMesh(MeshMain, EAGR_CombatColliderType::ComplexBoxTrace);
		}
		if (MeshSecondary)
		{
			CombatManager->AddTraceMesh(MeshSecondary, EAGR_CombatColliderType::ComplexBoxTrace);
		}
	}
	else
	{
		CombatManager->ClearAllMeshes();
	}
}

void AWOGBaseWeapon::IncreaseCombo()
{
	++ComboStreak;
	bAttackWindowOpen = true;
}

void AWOGBaseWeapon::ResetCombo()
{
	ComboStreak = 0;
	bAttackWindowOpen = false;
}

void AWOGBaseWeapon::SetOwnerCharacter(ABasePlayerCharacter* NewOwner)
{
	if (NewOwner)
	{
		OwnerCharacter = NewOwner;
	}
	UE_LOG(LogTemp, Warning, TEXT("New owner of weapon %s is : %s"), *GetNameSafe(this), *GetNameSafe(OwnerCharacter));
}
