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
#include "Components/SphereComponent.h"
#include "Data/AGRLibrary.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Data/WOGGameplayTags.h"
#include "Types/CharacterTypes.h"
#include "GameplayTags.h"
#include "AbilitySystem/Abilities/WOGGameplayAbilityBase.h"
#include "AbilitySystemComponent.h"
#include "Weapons/WOGRangedWeaponBase.h"
#include "Libraries/WOGBlueprintLibrary.h"
#include "PlayerController/WOGPlayerController.h"
#include "Resources/WOGCommonInventory.h"
#include "ActorComponents/WOGUIManagerComponent.h"
#include "GameplayTagContainer.h"

AWOGBaseWeapon::AWOGBaseWeapon()
{
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

	ComboStreak = 0;
}

void AWOGBaseWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWOGBaseWeapon, AbilityKey);
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
		ItemComponent->ItemTagSlotType = WeaponData.WeaponTag;
	}
}

void AWOGBaseWeapon::BeginPlay()
{
	Super::BeginPlay();
	OwnerCharacter = OwnerCharacter != nullptr ? OwnerCharacter : GetOwner() ? (TObjectPtr<ABasePlayerCharacter>) Cast<ABasePlayerCharacter>(GetOwner()) : nullptr;
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("NO OWNER CHARACTER AT BEGINPLAY"));
	}

	InitWeaponDefaults();

}

void AWOGBaseWeapon::InitData()
{
	OwnerCharacter = GetOwner() ? Cast<ABasePlayerCharacter>(GetOwner()) : nullptr;
	if (!ItemDataTable) return;

	TArray<FName> WeaponNamesArray = ItemDataTable->GetRowNames();
	FWeaponDataTable* WeaponDataRow = nullptr;

	for (auto WeaponRowName : WeaponNamesArray)
	{
		if (WeaponRowName == ItemNames[ItemLevel])
		{
			WeaponDataRow = ItemDataTable->FindRow<FWeaponDataTable>(ItemNames[ItemLevel], TEXT(""));
			break;
		}
	}
	if (WeaponDataRow)
	{
		MeshMain->SetStaticMesh(WeaponDataRow->MeshMain);
		MeshSecondary->SetStaticMesh(WeaponDataRow->MeshSecondary);
		
		WeaponDataRow->VendorItemData.ItemIcon = WeaponDataRow->AbilityIcon;
		WeaponDataRow->VendorItemData.ItemTag = WeaponDataRow->WeaponTag;
		WeaponDataRow->VendorItemData.BaseWeaponDamage = WeaponDataRow->BaseDamage;
		WeaponDataRow->VendorItemData.ComboMultiplier = WeaponDataRow->ComboDamageMultiplier;
		WeaponDataRow->VendorItemData.HeavyDamageMultiplier = WeaponDataRow->HeavyDamageMultiplier;
		WeaponDataRow->VendorItemData.WeaponStaminaCost = WeaponDataRow->Cost;
		WeaponDataRow->VendorItemData.bIsAttacker = WeaponDataRow->bIsAttacker;

		WeaponData = *WeaponDataRow;

		UpdateVendorData(WeaponDataRow);
	}
}

void AWOGBaseWeapon::UpdateVendorData(FWeaponDataTable* Row)
{
	if (Row)
	{
		WeaponData.VendorItemData.ItemIcon = Row->VendorItemData.ItemIcon;
		WeaponData.VendorItemData.ItemTag = Row->VendorItemData.ItemTag;
		WeaponData.VendorItemData.bIsAttacker = Row->VendorItemData.bIsAttacker;
		WeaponData.VendorItemData.BaseWeaponDamage = Row->VendorItemData.BaseWeaponDamage;
		WeaponData.VendorItemData.ComboMultiplier = Row->VendorItemData.ComboMultiplier;
		WeaponData.VendorItemData.HeavyDamageMultiplier = Row->VendorItemData.HeavyDamageMultiplier;
		WeaponData.VendorItemData.WeaponStaminaCost = Row->VendorItemData.WeaponStaminaCost;
	}
}

void AWOGBaseWeapon::AddAbilityWidget(const int32& Key)
{
	if (!OwnerCharacter || !OwnerCharacter->GetOwnerPC() || !OwnerCharacter->GetOwnerPC()->GetUIManagerComponent()) return;
	AbilityKey = Key;

	OwnerCharacter->GetOwnerPC()->GetUIManagerComponent()->Client_AddAbilityWidget(AbilityKey, WeaponData.AbilityWidgetClass, WeaponData.AbilityIcon, WeaponData.Cooldown, WeaponData.CooldownTag);
}

void AWOGBaseWeapon::InitWeaponDefaults()
{
	MeshMainOriginalTransform = MeshMain->GetRelativeTransform();
	MeshSecOriginalTransform = MeshSecondary->GetRelativeTransform();
}

void AWOGBaseWeapon::DropWeapon()
{
	if (!OwnerCharacter) return;
	RemoveGrantedAbilities(OwnerCharacter);

	UAGR_EquipmentManager* Equipment = UAGRLibrary::GetEquipment(OwnerCharacter);
	if (Equipment)
	{
		Equipment->RemoveWeaponShortcutReference(this);
	}

	ItemComponent->DropItem();

	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	SetActorLocation(
		OwnerCharacter->GetActorLocation() +
		(OwnerCharacter->GetActorForwardVector() * 200));

	MeshMain->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	MeshSecondary->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);

	MeshMain->SetRelativeTransform(MeshMainOriginalTransform);
	MeshSecondary->SetRelativeTransform(MeshSecOriginalTransform);

	SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SphereComponent->SetGenerateOverlapEvents(true);

	UAGRAnimMasterComponent* AnimMaster = UAGRLibrary::GetAnimationMaster(OwnerCharacter);
	if (AnimMaster)
	{
		AnimMaster->SetupBasePose(TAG_Pose_Relax);
	}

	if (OwnerCharacter->GetOwnerPC() && OwnerCharacter->GetOwnerPC()->GetUIManagerComponent())
	{
		OwnerCharacter->GetOwnerPC()->GetUIManagerComponent()->Client_RemoveAbilityWidget(AbilityKey);
	}

	OwnerCharacter = nullptr;
	SetOwner(nullptr);
}

void AWOGBaseWeapon::StoreWeapon(const FName& Key)
{
	if (!OwnerCharacter) return;

	RemoveGrantedAbilities(OwnerCharacter);

	//Unequip weapon from equipment manager
	UAGR_EquipmentManager* Equipment = UAGRLibrary::GetEquipment(OwnerCharacter);
	if (Equipment)
	{
		Equipment->RemoveWeaponShortcutReference(this);
		FText Note;
		Equipment->UnEquipByReference(this, Note);
	}

	MeshMain->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	MeshSecondary->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);

	MeshMain->SetRelativeTransform(MeshMainOriginalTransform);
	MeshSecondary->SetRelativeTransform(MeshSecOriginalTransform);

	SphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SphereComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SphereComponent->SetGenerateOverlapEvents(false);

	UAGRAnimMasterComponent* AnimMaster = UAGRLibrary::GetAnimationMaster(OwnerCharacter);
	if (AnimMaster)
	{
		AnimMaster->SetupBasePose(TAG_Pose_Relax);
	}

	if (OwnerCharacter->GetOwnerPC() && OwnerCharacter->GetOwnerPC()->GetUIManagerComponent())
	{
		OwnerCharacter->GetOwnerPC()->GetUIManagerComponent()->Client_RemoveAbilityWidget(AbilityKey);
	}

	if (Key == FName("1"))
	{
		ItemComponent->ItemAuxTag = TAG_Aux_Weapon_Primary;
	}
	else if (Key == FName("2"))
	{
		ItemComponent->ItemAuxTag = TAG_Aux_Weapon_Secondary;
	}

	// Last thing, clear OwnerCharacter
	OwnerCharacter = nullptr;
}

void AWOGBaseWeapon::RestoreWeapon(ABasePlayerCharacter* NewOwner)
{
	if (!HasAuthority() || !ItemComponent || !NewOwner) return;

	bool bIsActorAttacker = UWOGBlueprintLibrary::GetCharacterData(NewOwner).bIsAttacker;
	if (WeaponData.bIsAttacker != bIsActorAttacker) return;

	UAGR_EquipmentManager* Equipment = UAGRLibrary::GetEquipment(NewOwner);
	UAGR_InventoryManager* Inventory = UAGRLibrary::GetInventory(NewOwner);

	if (!Equipment || !Inventory)
	{
		UE_LOG(LogTemp, Error, TEXT("Equipment or Inventory not valid"));
		return;
	}

	FShortcutItemReference WeaponRef;
	int32 KeyInt = -1;

	if (ItemComponent)
	{
		if (ItemComponent->ItemAuxTag == TAG_Aux_Weapon_Primary)
		{
			KeyInt = 1;
		} 
		else if (ItemComponent->ItemAuxTag == TAG_Aux_Weapon_Secondary)
		{
			KeyInt = 2;
		}
	}

	WeaponRef.Key = *FString::FromInt(KeyInt);
	WeaponRef.ItemShortcut = this;

	if (Equipment->SaveWeaponShortcutReference(WeaponRef))
	{
		SetOwnerCharacter(NewOwner);
		SphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SphereComponent->SetGenerateOverlapEvents(false);
		ItemComponent->PickUpItem(Inventory);
		if (OwnerCharacter)
		{
			OwnerCharacter->Server_EquipWeapon(WeaponRef.Key, this);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Invalid OwnerCharacter"));
		}

		AddAbilityWidget(KeyInt);
		return;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Save weapon reference failed"));
	}
}

void AWOGBaseWeapon::OnWeaponOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority() || !ItemComponent) return;

	bool bIsActorAttacker = UWOGBlueprintLibrary::GetCharacterData(OtherActor).bIsAttacker;
	if (WeaponData.bIsAttacker != bIsActorAttacker) return;

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

	int32 MaxAmountWeapons = UWOGBlueprintLibrary::GetCharacterData(OtherActor).bIsAttacker ? 1 : 2;
	if (Equipment->WeaponShortcutReferences.Num() < MaxAmountWeapons)
	{
		FShortcutItemReference WeaponRef;
		int32 KeyInt = 1;

		if (Equipment->WeaponShortcutReferences.Num() == 1)
		{
			FName ExistingKey = Equipment->WeaponShortcutReferences[0].Key;
			KeyInt = ExistingKey == FName("1") ? 2 : 1;
		}

		WeaponRef.Key = *FString::FromInt(KeyInt);
		WeaponRef.ItemShortcut = this;

		if (Equipment->SaveWeaponShortcutReference(WeaponRef))
		{
			ABasePlayerCharacter* NewOwnerCharacter = Cast<ABasePlayerCharacter>(OtherActor);
			if (NewOwnerCharacter)
			{
				SetOwnerCharacter(NewOwnerCharacter);
			}
			SphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			SphereComponent->SetGenerateOverlapEvents(false);
			ItemComponent->PickUpItem(Inventory);
			if (OwnerCharacter)
			{
				OwnerCharacter->Server_EquipWeapon(WeaponRef.Key, this);
			}

			AddAbilityWidget(KeyInt);
			return;
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Too many weapons in the inventory"));
	}
}

void AWOGBaseWeapon::OnItemEquipped(AActor* User, FName SlotName)
{
	Multicast_OnWeaponEquip(User, SlotName);

	if (!User) return;
	UAGRAnimMasterComponent* AnimMaster = UAGRLibrary::GetAnimationMaster(User);
	if (!AnimMaster) return;

	if (SlotName == NAME_WeaponSlot_Primary)
	{
		AnimMaster->SetupBasePose(WeaponData.WeaponPoseTag);

		bool Success = GrantAbilities(User);
		UE_LOG(WOGLogCombat, Display, TEXT("WeaponGrantedAbilities applied: %d"), Success);
	}
	else
	{
		AnimMaster->SetupBasePose(TAG_Pose_Relax);

		bool Success = RemoveGrantedAbilities(User);
	}

	if (OwnerCharacter)
	{
		TObjectPtr<AWOGPlayerController> OwnerPC = Cast<AWOGPlayerController>(OwnerCharacter->GetController());
		if (OwnerPC && ItemComponent)
		{
			ItemComponent->PreviousOwnerIndex = OwnerPC->UserIndex;
			UE_LOG(WOGLogInventory, Display, TEXT("New PREVIOUS_USER_INDEX for weapon: %d"), ItemComponent->PreviousOwnerIndex);
		}
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

bool AWOGBaseWeapon::GrantAbilities(AActor* User)
{
	if (!HasAuthority() || WeaponData.Abilities.IsEmpty()) return false;
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(User);
	if (!ASC) return false;

	GrantedAbilities.Empty();

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
			CombatManager->TraceChannel = WeaponData.TraceChannel;
		}
		if (MeshSecondary)
		{
			CombatManager->AddTraceMesh(MeshSecondary, EAGR_CombatColliderType::ComplexBoxTrace);
			CombatManager->TraceChannel = WeaponData.TraceChannel;
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
	OwnerCharacter = OwnerCharacter == nullptr ? (TObjectPtr<ABasePlayerCharacter>) Cast<ABasePlayerCharacter>(GetOwner()) : OwnerCharacter;
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

	StartCatchRangedWeaponTimer();
}

void AWOGBaseWeapon::Server_SpawnWeaponAOE_Implementation(const FVector_NetQuantize& TargetLocation)
{
	OwnerCharacter = OwnerCharacter == nullptr ? (TObjectPtr<ABasePlayerCharacter>) Cast<ABasePlayerCharacter>(GetOwner()) : OwnerCharacter;
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
			TObjectPtr<AActor> SpawnedAOEAttack = World->SpawnActor<AActor>(WeaponData.RangedClass, SpawnTransform, SpawnParams);

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
	OwnerCharacter = OwnerCharacter == nullptr ? (TObjectPtr<ABasePlayerCharacter>) Cast<ABasePlayerCharacter>(GetOwner()) : OwnerCharacter;
	if (!OwnerCharacter) return;
	if (!OwnerCharacter->HasMatchingGameplayTag(TAG_State_Weapon_Ranged_Throw))
	{
		CatchRangedWeaponTimerHandle.Invalidate();
		return;
	}

	UAbilitySystemComponent* ASC = OwnerCharacter->GetAbilitySystemComponent();
	if (ASC)
	{
		FGameplayTagContainer EffectsToRemove;
		EffectsToRemove.AddTag(TAG_State_Weapon_Ranged_Throw);
		int32 EffectsRemoved = ASC->RemoveActiveEffectsWithTags(EffectsToRemove);
		UE_LOG(LogTemp, Display, TEXT("Ranged effect removed, %d"), EffectsRemoved);
	}

	MeshSecondary->SetVisibility(true);

	if (SpawnedRangedWeapon)
	{
		SpawnedRangedWeapon->Destroy();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to destroy ranged weapon"));
	}

}

void AWOGBaseWeapon::StartCatchRangedWeaponTimer()
{
	if (!HasAuthority()) return;

	float CatchDelay = 5.f;
	GetWorldTimerManager().SetTimer(CatchRangedWeaponTimerHandle, this, &ThisClass::CatchWeapon, CatchDelay, false);
}
