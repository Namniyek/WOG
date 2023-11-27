// Fill out your copyright notice in the Description page of Project Settings.


#include "Magic/WOGBaseMagic.h"
#include "WOG.h"
#include "Net/UnrealNetwork.h"
#include "WOG/PlayerCharacter/BasePlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Sound/SoundCue.h"
#include "Components/SphereComponent.h"
#include "Data/AGRLibrary.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Types/WOGGameplayTags.h"
#include "Types/CharacterTypes.h"
#include "GameplayTags.h"
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
	SphereComponent->bHiddenInGame = false;

	ItemComponent = CreateDefaultSubobject <UAGR_ItemComponent>(TEXT("ItemComponent"));

	StandbyEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Standby Effect"));
	StandbyEffect->SetupAttachment(GetRootComponent());
	StandbyEffect->bAutoActivate = true;
	StandbyEffect->SetIsReplicated(true);
}

void AWOGBaseMagic::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	InitMagicData();
}

void AWOGBaseMagic::InitMagicData()
{
	OwnerCharacter = GetOwner() ? Cast<ABasePlayerCharacter>(GetOwner()) : nullptr;

	const FString MagicTablePath{ TEXT("Engine.DataTable'/Game/Data/Magic/DT_Magic.DT_Magic'") };
	UDataTable* MagicTableObject = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *MagicTablePath));

	if (!MagicTableObject)
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid magic DataTable"));
		return;
	}

	TArray<FName> MagicNamesArray = MagicTableObject->GetRowNames();
	FMagicDataTable* MagicDataRow = nullptr;

	for (auto WeaponRowName : MagicNamesArray)
	{
		if (WeaponRowName == MagicName)
		{
			MagicDataRow = MagicTableObject->FindRow<FMagicDataTable>(MagicName, TEXT(""));
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
		ItemComponent->ItemName = MagicName;
		ItemComponent->ItemTagSlotType = MagicData.MagicTag;

		ItemComponent->OnPickup.AddDynamic(this, &ThisClass::OnMagicPickedUp);
		ItemComponent->OnEquip.AddDynamic(this, &ThisClass::OnMagicEquip);
		ItemComponent->OnUnEquip.AddDynamic(this, &ThisClass::OnMagicUnequip);
	}
}

void AWOGBaseMagic::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWOGBaseMagic, OwnerCharacter);
	DOREPLIFETIME(AWOGBaseMagic, AbilityKey);
}

void AWOGBaseMagic::BeginPlay()
{
	Super::BeginPlay();
	OwnerCharacter = OwnerCharacter != nullptr ? OwnerCharacter : GetOwner() ? (TObjectPtr<ABasePlayerCharacter>) Cast<ABasePlayerCharacter>(GetOwner()) : nullptr;
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
	}

	OwnerCharacter->GetOwnerPC()->GetUIManagerComponent()->Client_AddAbilityWidget(AbilityKey, MagicData.AbilityWidgetClass, MagicData.AbilityIcon, MagicData.Cooldown, MagicData.CooldownTag);
}

void AWOGBaseMagic::OnMagicPickedUp(UAGR_InventoryManager* Inventory)
{
	//Bound and kept just in case
}

void AWOGBaseMagic::OnMagicEquip(AActor* User, FName SlotName)
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
		LatentActionInfo.ExecutionFunction = "GrantMagicAbilities";
		LatentActionInfo.UUID = 123;
		LatentActionInfo.Linkage = 0;

		UKismetSystemLibrary::Delay(this, 0.05f, LatentActionInfo);
		SpawnIdleClass();

		Multicast_HandleStandbyCosmetics(false);
	}
	else
	{
		AnimMaster->SetupBasePose(TAG_Pose_Relax);

		bool Success = RemoveGrantedAbilities(User);

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

void AWOGBaseMagic::OnMagicUnequip(AActor* User, FName SlotName)
{
	//Bound and kept just in case
}

void AWOGBaseMagic::Multicast_OnMagicEquip_Implementation(AActor* User, FName SlotName)
{
	AttachToActor(User, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
}

bool AWOGBaseMagic::GrantMagicAbilities()
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

void AWOGBaseMagic::SpawnProjectile()
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

		GetBeamEndLocation(StartLocation, TraceHitResult);
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

void AWOGBaseMagic::Server_SpawnAOE_Implementation(const FVector_NetQuantize& TargetLocation)
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

bool AWOGBaseMagic::TraceUnderCrosshairs(FHitResult& OutHitResult, FVector& OutHitLocation)
{
	//Get Viewport size
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	//Get screen space location of the crosshair
	FVector2D CrosshairLocation((ViewportSize.X / 2.f), (ViewportSize.Y / 2.f));
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;

	//Get world position and direction of the crosshair
	TObjectPtr<APlayerController> PlayerController = Cast<APlayerController>(OwnerCharacter->GetController());
	if (!PlayerController) return false;

	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(PlayerController, CrosshairLocation, CrosshairWorldPosition, CrosshairWorldDirection);

	if (bScreenToWorld)
	{
		//Trace from Crosshair world location outward
		const FVector Start(CrosshairWorldPosition);
		const FVector End(Start + CrosshairWorldDirection * 50'000.f);
		OutHitLocation = End;
		GetWorld()->LineTraceSingleByChannel(OutHitResult, Start, End, ECollisionChannel::ECC_Visibility);

		if (OutHitResult.bBlockingHit)
		{
			OutHitLocation = OutHitResult.Location;
			return true;
		}
	}

	return false;
}

void AWOGBaseMagic::GetBeamEndLocation(const FVector& StartLocation, FHitResult& OutHitResult)
{
	//Check for crosshairs trace hit
	FHitResult CrosshairHitResult;
	FVector OutBeamLocation;
	bool bCrosshairHit(TraceUnderCrosshairs(CrosshairHitResult, OutBeamLocation));

	if (bCrosshairHit)
	{
		//Tentative beam location
		OutBeamLocation = CrosshairHitResult.Location;
	}

	//Perform a second trace from character barrel
	const FVector TraceStart = StartLocation;
	const FVector StartToEnd = OutBeamLocation - StartLocation;
	const FVector TraceEnd = OutBeamLocation; // + StartToEnd * 1.25f;

	GetWorld()->LineTraceSingleByChannel(OutHitResult, TraceStart, TraceEnd, ECollisionChannel::ECC_Visibility);

	//object between character and beam end point
	if (!OutHitResult.bBlockingHit)
	{
		OutHitResult.Location = OutBeamLocation;
	}
}

void AWOGBaseMagic::SetOwnerCharacter(ABasePlayerCharacter* NewOwner)
{
	if (NewOwner)
	{
		OwnerCharacter = NewOwner;
	}
	UE_LOG(WOGLogInventory, Warning, TEXT("New owner of magic %s is : %s"), *GetNameSafe(this), *GetNameSafe(OwnerCharacter));
}

