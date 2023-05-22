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
#include "Components/SphereComponent.h"
#include "Components/AGR_ItemComponent.h"
#include "Data/AGRLibrary.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Types/WOGGameplayTags.h"

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

	TraceComponent = CreateDefaultSubobject <UDidItHitActorComponent>(TEXT("TraceComponent"));


	ItemComponent = CreateDefaultSubobject <UAGR_ItemComponent>(TEXT("ItemComponent"));

}

void AWOGBaseWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWOGBaseWeapon, OwnerCharacter);
	DOREPLIFETIME(AWOGBaseWeapon, WeaponState);
	DOREPLIFETIME(AWOGBaseWeapon, bIsBlocking);
	DOREPLIFETIME(AWOGBaseWeapon, bCanParry);
	DOREPLIFETIME(AWOGBaseWeapon, bIsArmingHeavy);
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

	WeaponState = EWeaponState::EWS_Stored;
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
	if (!Inventory->GetOwner() || !HasAuthority()) return;
	UAGR_EquipmentManager* Equipment = UAGRLibrary::GetEquipment(Inventory->GetOwner());
	if (!Equipment) return;
	
	FName Key = FName("1");

	if (!Equipment->WeaponShortcutReferences.IsEmpty())
	{
		for (auto WeaponRef : Equipment->WeaponShortcutReferences)
		{
			if (WeaponRef.ItemShortcut == this)
			{
				Key = WeaponRef.Key;
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("No saved reference shortcut"));
		return;
	}
	
	AActor* OutItem = nullptr;
	if (Key == FName("1") && !Equipment->GetItemInSlot(FName("BackMain"), OutItem))
	{
		AActor* PreviousItem = nullptr;
		AActor* NewItem = nullptr;
		bool bSucess = Equipment->EquipItemInSlot(FName("BackMain"), this, PreviousItem, NewItem);
		UE_LOG(LogTemp, Display, TEXT("Equipped to BackMain : %d"), bSucess);
		return;
	}
	if (Key == FName("2") && !Equipment->GetItemInSlot(FName("BackSecondary"), OutItem))
	{
		AActor* PreviousItem = nullptr;
		AActor* NewItem = nullptr;
		bool bSucess = Equipment->EquipItemInSlot(FName("BackSecondary"), this, PreviousItem, NewItem);
		UE_LOG(LogTemp, Display, TEXT("Equipped to BackSecondary : %d"), bSucess);
		return;
	}
	UE_LOG(LogTemp, Error, TEXT("Not equipped at all"));
}

void AWOGBaseWeapon::OnWeaponEquip(AActor* User, FName SlotName)
{
	if (!User) return;
	Multicast_OnWeaponEquip(User, SlotName);
}

void AWOGBaseWeapon::Multicast_OnWeaponEquip_Implementation(AActor* User, FName SlotName)
{
	if (!User) return;

	if (SlotName == FName("Primary"))
	{
		FGameplayEventData EventPayload;
		EventPayload.EventTag = TAG_Event_Weapon_Equip;
		EventPayload.OptionalObject = this;
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(User, TAG_Event_Weapon_Equip, EventPayload);
		return;
	}
	if (SlotName == FName("BackMain") || SlotName == FName("BackSecondary"))
	{
		FGameplayEventData EventPayload;
		EventPayload.EventTag = TAG_Event_Weapon_Unequip;
		EventPayload.OptionalObject = this;
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(User, TAG_Event_Weapon_Unequip, EventPayload);
		return;
	}
}

void AWOGBaseWeapon::OnWeaponUnequip(AActor* User, FName SlotName)
{
	//Empty for now
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
		break;
	case EWeaponState::EWS_BeingEquipped:
		Equip();
		break;
	case EWeaponState::EWS_BeingStored:
		Unequip();
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
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("No OWNER CHARACTER"));
		return;
	}

	UAnimInstance* CharacterAnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
	if (CharacterAnimInstance && WeaponData.EquipMontage)
	{
		CharacterAnimInstance->Montage_Play(WeaponData.EquipMontage, 1.f);
		CharacterAnimInstance->Montage_JumpToSection(FName("Equip"), WeaponData.EquipMontage);
	}

	AttachToHands();
}

void AWOGBaseWeapon::AttachToHands()
{
	if (!OwnerCharacter) return;
	AttachToActor(OwnerCharacter, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

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

	MeshMain->AttachToComponent(OwnerCharacter->GetMesh(), AttachmentRules, WeaponData.MeshMainSocket);
	MeshSecondary->AttachToComponent(OwnerCharacter->GetMesh(), AttachmentRules, WeaponData.MeshSecondarySocket);

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
	if (CharacterAnimInstance && WeaponData.EquipMontage)
	{
		CharacterAnimInstance->Montage_Play(WeaponData.EquipMontage, 1.f);
		CharacterAnimInstance->Montage_JumpToSection(FName("Unequip"), WeaponData.EquipMontage);
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
	UE_LOG(LogTemp, Error, TEXT("AttachToBack"));
	AttachToActor(OwnerCharacter, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

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

	MeshMain->AttachToComponent(OwnerCharacter->GetMesh(), AttachmentRules, WeaponData.BackMainSocket);
	MeshSecondary->AttachToComponent(OwnerCharacter->GetMesh(), AttachmentRules, WeaponData.BackSecondarySocket);

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

void AWOGBaseWeapon::AttackLight()
{
	if (!OwnerCharacter) return;
	if (HasAuthority())
	{
		bIsArmingHeavy = false;
		SetWeaponState(EWeaponState::EWS_AttackLight);
		bAttackWindowOpen = false;
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
	}
}

void AWOGBaseWeapon::AttackHeavy()
{
	if (!OwnerCharacter) return;
	if (HasAuthority())
	{
		bIsArmingHeavy = false;
		SetWeaponState(EWeaponState::EWS_AttackHeavy);
		bAttackWindowOpen = false;
	}

	ComboStreak = 0;

	if (TraceComponent)
	{
		TraceComponent->ToggleTraceCheck(false);
		HitActorsToIgnore.Empty();
	}
}

void AWOGBaseWeapon::Block()
{
	if (!OwnerCharacter || !HasAuthority()) return;

	SetWeaponState(EWeaponState::EWS_Blocking);
	bCanParry = true;
	GetWorldTimerManager().SetTimer(ParryTimer, this, &ThisClass::SetCanNotParry, WeaponData.MaxParryThreshold);
}

void AWOGBaseWeapon::StopBlocking()
{
	if (!OwnerCharacter || !HasAuthority()) return;
	SetWeaponState(EWeaponState::EWS_Equipped);
}

void AWOGBaseWeapon::HitDetected(FHitResult Hit)
{
	if (!Hit.bBlockingHit || !Hit.GetActor()) return;

	float DamageToApply = 0.f;
	switch (WeaponState)
	{
	case EWeaponState::EWS_AttackLight:
		DamageToApply = WeaponData.BaseDamage + (WeaponData.BaseDamage * WeaponData.DamageMultiplier) + (WeaponData.BaseDamage * (ComboStreak * WeaponData.ComboDamageMultiplier));
		break;
	case EWeaponState::EWS_AttackHeavy:
		DamageToApply = WeaponData.BaseDamage + (WeaponData.BaseDamage * WeaponData.DamageMultiplier) + (WeaponData.BaseDamage * WeaponData.HeavyDamageMultiplier);
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


