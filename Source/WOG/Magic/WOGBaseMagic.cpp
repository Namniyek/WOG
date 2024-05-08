// Fill out your copyright notice in the Description page of Project Settings.


#include "Magic/WOGBaseMagic.h"
#include "WOG.h"
#include "Net/UnrealNetwork.h"
#include "WOG/PlayerCharacter/BasePlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/SphereComponent.h"
#include "Data/AGRLibrary.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Data/WOGGameplayTags.h"
#include "Types/CharacterTypes.h"
#include "AbilitySystem/Abilities/WOGGameplayAbilityBase.h"
#include "AbilitySystemComponent.h"
#include "Magic/WOGBaseIdleMagic.h"
#include "Magic/Projectile/WOGBaseMagicProjectile.h"
#include "Magic/AOE/WOGBaseMagicAOE.h"
#include "Libraries/WOGBlueprintLibrary.h"
#include "NiagaraComponent.h"
#include "PlayerController/WOGPlayerController.h"
#include "ActorComponents/WOGUIManagerComponent.h"

AWOGBaseMagic::AWOGBaseMagic()
{
	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapSphere"));
	SetRootComponent(SphereComponent);
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SphereComponent->InitSphereRadius(96.f);
	SphereComponent->SetGenerateOverlapEvents(true);
	SphereComponent->bHiddenInGame = false;

	StandbyEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Standby Effect"));
	StandbyEffect->SetupAttachment(GetRootComponent());
	StandbyEffect->bAutoActivate = true;
	StandbyEffect->SetIsReplicated(true);
}

void AWOGBaseMagic::InitData()
{
	OwnerCharacter = GetOwner() ? Cast<ABasePlayerCharacter>(GetOwner()) : nullptr;
	if (!ItemDataTable)
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid magic DataTable"));
		return;
	}

	TArray<FName> MagicNamesArray = ItemDataTable->GetRowNames();
	FMagicDataTable* MagicDataRow = nullptr;

	for (auto WeaponRowName : MagicNamesArray)
	{
		if (WeaponRowName == ItemNames[ItemLevel])
		{
			MagicDataRow = ItemDataTable->FindRow<FMagicDataTable>(ItemNames[ItemLevel], TEXT(""));
			break;
		}
	}
	if (MagicDataRow)
	{
		MagicDataRow->VendorItemData.ItemIcon = MagicDataRow->AbilityIcon;
		MagicDataRow->VendorItemData.ItemTag = MagicDataRow->MagicTag;
		MagicDataRow->VendorItemData.BaseMagicValue = MagicDataRow->Value;
		MagicDataRow->VendorItemData.MagicCooldown = MagicDataRow->Cooldown;
		MagicDataRow->VendorItemData.MagicDuration = MagicDataRow->Duration;
		MagicDataRow->VendorItemData.CastCost = MagicDataRow->Cost;
		MagicDataRow->VendorItemData.bIsAttacker = MagicDataRow->bIsAttacker;

		MagicData = *MagicDataRow;
	}
}

void AWOGBaseMagic::UpdateVendorData(FMagicDataTable* Row)
{
	if (Row)
	{
		MagicData.VendorItemData.ItemIcon = Row->VendorItemData.ItemIcon;
		MagicData.VendorItemData.ItemTag = Row->VendorItemData.ItemTag;
		MagicData.VendorItemData.bIsAttacker = Row->VendorItemData.bIsAttacker;
		MagicData.VendorItemData.BaseMagicValue = Row->VendorItemData.BaseMagicValue;
		MagicData.VendorItemData.MagicCooldown = Row->VendorItemData.MagicCooldown;
		MagicData.VendorItemData.MagicDuration = Row->VendorItemData.MagicDuration;
		MagicData.VendorItemData.CastCost = Row->VendorItemData.CastCost;
	}
}

void AWOGBaseMagic::SpawnIdleClass()
{
	OwnerCharacter = OwnerCharacter == nullptr ? (TObjectPtr<ABasePlayerCharacter>) Cast<ABasePlayerCharacter>(GetOwner()) : OwnerCharacter;
	if (!OwnerCharacter) return;
	if (MagicData.AbilityType != EAbilityType::EAT_Projectile && MagicData.AbilityType != EAbilityType::EAT_AOE) return;

	TSubclassOf<AWOGBaseIdleMagic> IdleClass;

	switch (MagicData.AbilityType)
	{
	case EAbilityType::EAT_Projectile:
		if (UKismetSystemLibrary::IsValidClass(MagicData.IdleProjectileClass))
		{
			IdleClass = MagicData.IdleProjectileClass;
		}
		break;

	case EAbilityType::EAT_AOE:
		if (UKismetSystemLibrary::IsValidClass(MagicData.IdleAOEClass))
		{
			IdleClass = MagicData.IdleAOEClass;
		}
		break;
	default:
		break;
	}

	if (UKismetSystemLibrary::IsValidClass(IdleClass))
	{
		FTransform SpawnTransform = FTransform();
		SpawnTransform.SetLocation(OwnerCharacter->GetActorLocation());

		//Spawn Idle class
		UWorld* World = GetWorld();
		if (World)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			SpawnParams.Owner = OwnerCharacter;
			
			IdleActor = World->SpawnActor<AWOGBaseIdleMagic>(IdleClass, SpawnTransform, SpawnParams);

			if (IdleActor)
			{
				IdleActor->AttachToActor(OwnerCharacter, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
				UE_LOG(LogTemp, Display, TEXT("Idle class spawned"));
			}
		}
	}
}

void AWOGBaseMagic::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (SphereComponent && HasAuthority())
	{
		SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnMagicOverlap);
	}

	if (ItemComponent && HasAuthority())
	{
		ItemComponent->ItemTagSlotType = MagicData.MagicTag;
	}
}

void AWOGBaseMagic::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWOGBaseMagic, AbilityKey);
}

void AWOGBaseMagic::OnMagicOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority() || !ItemComponent || !OtherActor) return;

	bool bIsActorAttacker = UWOGBlueprintLibrary::GetCharacterData(OtherActor).bIsAttacker;
	if (MagicData.bIsAttacker != bIsActorAttacker) return;

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

	int32 MaxAmountMagics = UWOGBlueprintLibrary::GetCharacterData(OtherActor).bIsAttacker ? 2 : 1;
	if (Equipment->MagicShortcutReferences.Num() < MaxAmountMagics)
	{
		FShortcutItemReference MagicRef;
		int32 KeyInt = 1;

		if (Equipment->MagicShortcutReferences.Num() == 1)
		{
			FName ExistingKey = Equipment->MagicShortcutReferences[0].Key;
			KeyInt = ExistingKey == FName("1") ? 2 : 1;
		}

		MagicRef.Key = *FString::FromInt(KeyInt);
		MagicRef.ItemShortcut = this;

		if (Equipment->SaveMagicShortcutReference(MagicRef))
		{
			ABasePlayerCharacter* NewOwnerCharacter = Cast<ABasePlayerCharacter>(OtherActor);
			if (NewOwnerCharacter)
			{
				SetOwnerCharacter(NewOwnerCharacter);
			}
			SphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			SphereComponent->SetGenerateOverlapEvents(false);
			ItemComponent->PickUpItem(Inventory);

			Multicast_HandleStandbyCosmetics(false);

			if (OwnerCharacter)
			{
				OwnerCharacter->Server_EquipMagic(MagicRef.Key, this);
				AttachToActor(OwnerCharacter, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			}

			AddAbilityWidget(KeyInt);
			return;
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Too many magics in the inventory"));
	}
}

void AWOGBaseMagic::AddAbilityWidget(const int32& Key)
{
	if (!OwnerCharacter || !OwnerCharacter->GetOwnerPC() || !OwnerCharacter->GetOwnerPC()->GetUIManagerComponent()) return;

	switch (Key)
	{
	case 1:
		AbilityKey = OwnerCharacter->GetCharacterData().bIsAttacker ? 2 : 3;
		break;

	case 2:
		AbilityKey = OwnerCharacter->GetCharacterData().bIsAttacker ? 3 : -1;
		break;
	default:
		break;
	}

	OwnerCharacter->GetOwnerPC()->GetUIManagerComponent()->Client_AddAbilityWidget(AbilityKey, MagicData.AbilityWidgetClass, MagicData.AbilityIcon, MagicData.Cooldown, MagicData.CooldownTag);
}

void AWOGBaseMagic::OnItemEquipped(AActor* User, FName SlotName)
{
	if (!User) return;

	Multicast_OnMagicEquip(User, SlotName);

	UAGRAnimMasterComponent* AnimMaster = UAGRLibrary::GetAnimationMaster(User);
	if (!AnimMaster) return;

	if (SlotName == NAME_MagicSlot_MagicPrimary)
	{
		AnimMaster->SetupBasePose(MagicData.MagicPoseTag);

		FLatentActionInfo LatentActionInfo;
		LatentActionInfo.CallbackTarget = this;
		LatentActionInfo.ExecutionFunction = "GrantAbilities";
		LatentActionInfo.UUID = 123;
		LatentActionInfo.Linkage = 0;

		UKismetSystemLibrary::Delay(this, 0.05f, LatentActionInfo);
		SpawnIdleClass();

		Multicast_HandleStandbyCosmetics(false);
	}
	else
	{
		AnimMaster->SetupBasePose(TAG_Pose_Relax);

		RemoveGrantedAbilities(User);

		if (IdleActor)
		{
			IdleActor->Destroy();
		}

		Multicast_HandleStandbyCosmetics(false);
	}

	if (OwnerCharacter)
	{
		TObjectPtr<AWOGPlayerController> OwnerPC = Cast<AWOGPlayerController>(OwnerCharacter->GetController());
		if (OwnerPC && ItemComponent)
		{
			ItemComponent->PreviousOwnerIndex = OwnerPC->UserIndex;
			UE_LOG(WOGLogInventory, Display, TEXT("New PREVIOUS_USER_INDEX for magic: %d"), ItemComponent->PreviousOwnerIndex);
		}
	}
}

void AWOGBaseMagic::Multicast_OnMagicEquip_Implementation(AActor* User, FName SlotName)
{
	AttachToActor(User, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
}

bool AWOGBaseMagic::GrantAbilities(AActor* User)
{
	if (!HasAuthority() || MagicData.AbilitiesToGrant.IsEmpty()) return false;
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerCharacter);
	if (!ASC) return false;
	for (auto Ability : MagicData.AbilitiesToGrant)
	{
		if (!Ability) continue;
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(Ability, 1, static_cast<int32>(Ability.GetDefaultObject()->AbilityInputID), OwnerCharacter);
		FGameplayAbilitySpecHandle GrantedAbility = ASC->GiveAbility(AbilitySpec);
		GrantedAbilities.AddUnique(GrantedAbility);
		UE_LOG(LogTemp, Display, TEXT("Ability granted: %s"), *Ability.GetDefaultObject()->GetName());
	}
	return true;
}

bool AWOGBaseMagic::RemoveGrantedAbilities(AActor* User)
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

void AWOGBaseMagic::StoreMagic(const FName& Key)
{
	if (!OwnerCharacter) return;

	RemoveGrantedAbilities(OwnerCharacter);

	//Unequip weapon from equipment manager
	UAGR_EquipmentManager* Equipment = UAGRLibrary::GetEquipment(OwnerCharacter);
	if (Equipment)
	{
		Equipment->RemoveMagicShortcutReference(this);
		FText Note;
		Equipment->UnEquipByReference(this, Note);
	}

	/*MeshMain->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	MeshSecondary->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);

	MeshMain->SetRelativeTransform(MeshMainOriginalTransform);
	MeshSecondary->SetRelativeTransform(MeshSecOriginalTransform);*/

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

	//TO-DO Sort keys
	if (Key == FName("1"))
	{
		ItemComponent->ItemAuxTag = TAG_Aux_Magic_Primary;
	}
	else if (Key == FName("2"))
	{
		ItemComponent->ItemAuxTag = TAG_Aux_Magic_Secondary;
	}

	// Last thing, clear OwnerCharacter
	OwnerCharacter = nullptr;
}

void AWOGBaseMagic::RestoreMagic(ABasePlayerCharacter* NewOwner)
{
	if (!HasAuthority() || !ItemComponent || !NewOwner)
	{
		UE_LOG(WOGLogInventory, Error, TEXT("!HasAuthority() || !ItemComponent || !NewOwner"));
		return;
	}

	bool bIsActorAttacker = UWOGBlueprintLibrary::GetCharacterData(NewOwner).bIsAttacker;
	if (MagicData.bIsAttacker != bIsActorAttacker)
	{
		UE_LOG(WOGLogInventory, Error, TEXT("MagicData.bIsAttacker != bIsActorAttacker"));
		return;
	}

	UAGR_EquipmentManager* Equipment = UAGRLibrary::GetEquipment(NewOwner);
	UAGR_InventoryManager* Inventory = UAGRLibrary::GetInventory(NewOwner);

	if (!Equipment || !Inventory)
	{
		UE_LOG(WOGLogInventory, Error, TEXT("Equipment or Inventory not valid"));
		return;
	}

	FShortcutItemReference MagicRef;
	int32 KeyInt = -1;

	if (ItemComponent)
	{
		if (ItemComponent->ItemAuxTag == TAG_Aux_Magic_Primary)
		{
			KeyInt = 1;
		}
		else if (ItemComponent->ItemAuxTag == TAG_Aux_Magic_Secondary)
		{
			KeyInt = 2;
		}
	}

	MagicRef.Key = *FString::FromInt(KeyInt);
	MagicRef.ItemShortcut = this;

	if (Equipment->SaveMagicShortcutReference(MagicRef))
	{
		SetOwnerCharacter(NewOwner);
		SphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SphereComponent->SetGenerateOverlapEvents(false);
		ItemComponent->PickUpItem(Inventory);
		if (OwnerCharacter)
		{
			OwnerCharacter->Server_EquipMagic(MagicRef.Key, this);
		}
		else
		{
			UE_LOG(WOGLogInventory, Error, TEXT("Invalid OwnerCharacter"));
		}

		AddAbilityWidget(KeyInt);
		return;
	}
	else
	{
		UE_LOG(WOGLogInventory, Error, TEXT("Save magic reference failed"));
	}
}

void AWOGBaseMagic::DropMagic()
{
	if (!OwnerCharacter) return;
	RemoveGrantedAbilities(OwnerCharacter);

	UAGR_EquipmentManager* Equipment = UAGRLibrary::GetEquipment(OwnerCharacter);
	if (Equipment)
	{
		bool bSuccess = Equipment->RemoveMagicShortcutReference(this);
		UE_LOG(LogTemp, Warning, TEXT("Dropped magic: %d"), bSuccess);
	}

	ItemComponent->DropItem();

	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	SetActorLocation(
		OwnerCharacter->GetActorLocation() +
		(OwnerCharacter->GetActorForwardVector() * 200));

	if (IdleActor)
	{
		IdleActor->Destroy();
	}

	SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SphereComponent->SetGenerateOverlapEvents(true);

	Multicast_HandleStandbyCosmetics(true);

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

void AWOGBaseMagic::Multicast_HandleStandbyCosmetics_Implementation(bool NewEnabled)
{
	if (NewEnabled)
	{
		StandbyEffect->Activate(true);
	}
	else
	{
		StandbyEffect->DeactivateImmediate();
	}
}

void AWOGBaseMagic::PrepareProjectileSpawn()
{
	if (!OwnerCharacter || !OwnerCharacter->IsLocallyControlled()) return;
	if (MagicData.AbilityType != EAbilityType::EAT_Projectile) return;
	if (!UKismetSystemLibrary::IsValidClass(MagicData.ProjectileClass)) return;

	FVector StartLocation = OwnerCharacter->GetActorLocation() + OwnerCharacter->GetActorForwardVector()*50.f;
	FVector EndLocation;
	if (OwnerCharacter->GetCurrentTarget())
	{
		EndLocation = OwnerCharacter->GetCurrentTarget()->GetActorLocation();
	}
	else
	{
		FHitResult TraceHitResult;

		OwnerCharacter->GetBeamEndLocation(StartLocation, TraceHitResult);
		EndLocation = TraceHitResult.Location;
	}

	FRotator StartRotation = (EndLocation - StartLocation).GetSafeNormal().Rotation();
	
	FTransform SpawnTransform;
	SpawnTransform.SetRotation(FQuat::MakeFromRotator(StartRotation));
	SpawnTransform.SetLocation(StartLocation);

	Server_SpawnProjectile(SpawnTransform);
}

void AWOGBaseMagic::Server_SpawnProjectile_Implementation(const FTransform& SpawnTransform)
{
	if (!OwnerCharacter) return;
	if (!UKismetSystemLibrary::IsValidClass(MagicData.ProjectileClass)) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = OwnerCharacter;
	TObjectPtr<AWOGBaseMagicProjectile> SpawnedProjectile = GetWorld()->SpawnActor<AWOGBaseMagicProjectile>(MagicData.ProjectileClass, SpawnTransform, SpawnParams);
	if (SpawnedProjectile)
	{
		SpawnedProjectile->SetMagicData(MagicData);
	}
}

void AWOGBaseMagic::SpawnAOE(const FVector_NetQuantize& TargetLocation)
{
	OwnerCharacter = OwnerCharacter == nullptr ? (TObjectPtr<ABasePlayerCharacter>) Cast<ABasePlayerCharacter>(GetOwner()) : OwnerCharacter;
	if (!OwnerCharacter) return;

	//Set the correct transform
	if (UKismetSystemLibrary::IsValidClass(MagicData.AOEClass))
	{
		FTransform SpawnTransform = FTransform();
		SpawnTransform.SetLocation(TargetLocation);

		//Spawn Ranged Weapon
		UWorld* World = GetWorld();
		if (World)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			SpawnParams.Owner = OwnerCharacter;
			TObjectPtr<AWOGBaseMagicAOE> SpawnedAOEAttack = World->SpawnActor<AWOGBaseMagicAOE>(MagicData.AOEClass, SpawnTransform, SpawnParams);

			if (SpawnedAOEAttack)
			{
				SpawnedAOEAttack->SetMagicData(MagicData);
				UE_LOG(LogTemp, Warning, TEXT("AOE spawned: %s"), *GetNameSafe(SpawnedAOEAttack));
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Magic AOE class invalid"));
		return;
	}
}
