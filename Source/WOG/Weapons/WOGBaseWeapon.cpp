// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGBaseWeapon.h"
#include "WOG.h"
#include "Net/UnrealNetwork.h"
#include "WOG/PlayerCharacter/BasePlayerCharacter.h"
#include "WOG/Enemies/WOGBaseEnemy.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Sound/SoundCue.h"
#include "WOG/Interfaces/BuildingInterface.h"
#include "WOG/Interfaces/AttributesInterface.h"
#include "Components/SphereComponent.h"
#include "Data/AGRLibrary.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Types/WOGGameplayTags.h"
#include "GameplayTags.h"
#include "AbilitySystem/Abilities/WOGGameplayAbilityBase.h"
#include "AbilitySystemComponent.h"
#include "Weapons/WOGRangedWeaponBase.h"

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
		WeaponData.StunDuration = WeaponDataRow->StunDuration;

		WeaponData.SwingSound = WeaponDataRow->SwingSound;
		WeaponData.HitSound = WeaponDataRow->HitSound;
		WeaponData.BlockSound = WeaponDataRow->BlockSound;

		WeaponData.MeshMainSocket = WeaponDataRow->MeshMainSocket;
		WeaponData.MeshSecondarySocket = WeaponDataRow->MeshSecondarySocket;
		WeaponData.BackMainSocket = WeaponDataRow->BackMainSocket;
		WeaponData.BackSecondarySocket = WeaponDataRow->BackSecondarySocket;

		WeaponData.WeaponDamageEffect = WeaponDataRow->WeaponDamageEffect;
		WeaponData.RangedWeaponEffect = WeaponDataRow->RangedWeaponEffect;

		WeaponData.Abilities = WeaponDataRow->Abilities;
		WeaponData.BlockImpactLightTags = WeaponDataRow->BlockImpactLightTags;
		WeaponData.BlockImpactHeavyTag = WeaponDataRow->BlockImpactHeavyTag;
		WeaponData.ParryTag = WeaponDataRow->ParryTag;
		WeaponData.RangedTag = WeaponDataRow->RangedTag;
		WeaponData.AnimationSpeed = WeaponDataRow->AnimationSpeed;
		WeaponData.RangedClass = WeaponDataRow->RangedClass;
	}

	ComboStreak = 0;
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

		bool Success = GrantWeaponAbilities(User);
		UE_LOG(LogTemp, Display, TEXT("WeaponGrantedAbilities applied: %d"), Success);
	}
	else
	{
		AnimMaster->SetupBasePose(TAG_Pose_Relax);

		bool Success = RemoveGrantedAbilities(User);
		UE_LOG(LogTemp, Display, TEXT("WeaponAbilities removed: %d"), Success);
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

bool AWOGBaseWeapon::GrantWeaponAbilities(AActor* User)
{
	if (!HasAuthority() || WeaponData.Abilities.IsEmpty()) return false;
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(User);
	if (!ASC) return false;
	for (auto Ability : WeaponData.Abilities)
	{
		if (!Ability) continue;
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(Ability, 1, static_cast<int32>(Ability.GetDefaultObject()->AbilityInputID), User);
		FGameplayAbilitySpecHandle GrantedAbility = ASC->GiveAbility(AbilitySpec);
		GrantedAbilities.AddUnique(GrantedAbility);
		UE_LOG(LogTemp, Display, TEXT("Ability granted: %s"), *Ability.GetDefaultObject()->GetName());
	}
	return true;
}

bool AWOGBaseWeapon::RemoveGrantedAbilities(AActor* User)
{
	if (!HasAuthority() || GrantedAbilities.IsEmpty()) return false;
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(User);
	if (!ASC) return false;
	for (auto GrantedAbility : GrantedAbilities)
	{
		ASC->ClearAbility(GrantedAbility);
		UE_LOG(LogTemp, Display, TEXT("Ability cleared: %s"), *GrantedAbility.ToString())
	}
	return true;
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
	// Kept here just in case...
}

void AWOGBaseWeapon::AttackHeavy()
{
	// Kept here just in case...	
}

void AWOGBaseWeapon::Block()
{
	//Here just in case
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
}

void AWOGBaseWeapon::ResetCombo()
{
	ComboStreak = 0;
}


void AWOGBaseWeapon::Server_ThrowWeapon_Implementation(bool IsTargetValid, const FVector_NetQuantize& TargetLocation)
{
	OwnerCharacter = OwnerCharacter == nullptr ? Cast<ABasePlayerCharacter>(GetOwner()) : OwnerCharacter;
	if (!OwnerCharacter) return;

	//Spawn throwable weapon actor
	if (UKismetSystemLibrary::IsValidClass(WeaponData.RangedClass))
	{
		FTransform SpawnTransform = FTransform();
		FVector HandLocation = OwnerCharacter->GetMesh()->GetSocketLocation(FName("Hand_L"));
		SpawnTransform.SetLocation(HandLocation);

		if (IsTargetValid)
		{
			//Current target is valid. Throw weapon at target
			FVector Direction;
			Direction = TargetLocation - HandLocation;

			SpawnTransform.SetRotation(Direction.ToOrientationQuat());
		}
		else
		{
			//No target, throw weapon to where the camera is pointing
			SpawnTransform.SetRotation(OwnerCharacter->GetControlRotation().Quaternion());
		}

		//Spawn Ranged Weapon
		UWorld* World = GetWorld();
		if (World)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			SpawnParams.Owner = OwnerCharacter;
			SpawnedRangedWeapon = World->SpawnActor<AWOGRangedWeaponBase>(WeaponData.RangedClass, SpawnTransform, SpawnParams);

			if (SpawnedRangedWeapon)
			{
				UE_LOG(LogTemp, Display, TEXT("Ranged Weapon %s spawned"), *SpawnedRangedWeapon.GetName());
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Throwable class invalid"));
		return;
	}

	UAbilitySystemComponent* ASC = OwnerCharacter->GetAbilitySystemComponent();
	if (ASC)
	{
		FGameplayEffectContextHandle RangedContext = ASC->MakeEffectContext();
		FGameplayEffectSpecHandle OutSpec = ASC->MakeOutgoingSpec(WeaponData.RangedWeaponEffect, 1, RangedContext);
		ASC->ApplyGameplayEffectSpecToSelf(*OutSpec.Data);
		UE_LOG(LogTemp, Display, TEXT("Ranged effect applied"));
	}

	MeshSecondary->SetVisibility(false);
}

void AWOGBaseWeapon::Server_SpawnWeaponAOE_Implementation(const FVector_NetQuantize& TargetLocation)
{
	OwnerCharacter = OwnerCharacter == nullptr ? Cast<ABasePlayerCharacter>(GetOwner()) : OwnerCharacter;
	if (!OwnerCharacter) return;

	//Spawn AOE weapon actor
	if (UKismetSystemLibrary::IsValidClass(WeaponData.RangedClass))
	{
		FTransform SpawnTransform = FTransform();
		SpawnTransform.SetLocation(TargetLocation);
		
		FRotator Rotation = OwnerCharacter->GetControlRotation();
		Rotation.Pitch = 0.f;
		Rotation.Roll = 0.f;
		SpawnTransform.SetRotation(Rotation.Quaternion());

		//Spawn Ranged Weapon
		UWorld* World = GetWorld();
		if (World)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			SpawnParams.Owner = OwnerCharacter;
			SpawnedAOEAttack = World->SpawnActor<AActor>(WeaponData.RangedClass, SpawnTransform, SpawnParams);

			if (SpawnedAOEAttack)
			{
				UE_LOG(LogTemp, Display, TEXT("AOE Attack %s spawned"), *SpawnedAOEAttack.GetName());
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Throwable class invalid"));
		return;
	}
}

void AWOGBaseWeapon::RecallWeapon()
{
	if (SpawnedRangedWeapon)
	{
		SpawnedRangedWeapon->RecallWeapon();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to recall weapon"));
	}
}

void AWOGBaseWeapon::CatchWeapon()
{
	OwnerCharacter = OwnerCharacter == nullptr ? Cast<ABasePlayerCharacter>(GetOwner()) : OwnerCharacter;
	if (!OwnerCharacter) return;

	UE_LOG(LogTemp, Warning, TEXT("Running Catch ability c++"));

	UAbilitySystemComponent* ASC = OwnerCharacter->GetAbilitySystemComponent();
	if (ASC)
	{
		FGameplayTagContainer EffectsToRemove;
		EffectsToRemove.AddTag(TAG_State_Weapon_Ranged);
		ASC->RemoveActiveEffectsWithTags(EffectsToRemove);
		UE_LOG(LogTemp, Warning, TEXT("Ranged effect removed"));
	}

	MeshSecondary->SetVisibility(true);

	if (SpawnedRangedWeapon)
	{
		SpawnedRangedWeapon->Destroy();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to destroy weapon"));
	}

}

void AWOGBaseWeapon::SetOwnerCharacter(ABasePlayerCharacter* NewOwner)
{
	if (NewOwner)
	{
		OwnerCharacter = NewOwner;
	}
	UE_LOG(LogTemp, Warning, TEXT("New owner of weapon %s is : %s"), *GetNameSafe(this), *GetNameSafe(OwnerCharacter));
}
