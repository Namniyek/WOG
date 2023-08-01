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
#include "GameplayTags.h"
#include "AbilitySystem/Abilities/WOGGameplayAbilityBase.h"
#include "AbilitySystemComponent.h"
#include "Magic/WOGBaseIdleMagic.h"

AWOGBaseMagic::AWOGBaseMagic()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
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
		MagicData = *MagicDataRow;
	}
}

void AWOGBaseMagic::SpawnIdleClass()
{
	OwnerCharacter = OwnerCharacter == nullptr ? Cast<ABasePlayerCharacter>(GetOwner()) : OwnerCharacter;
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

		//Spawn Ranged Weapon
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
}

void AWOGBaseMagic::BeginPlay()
{
	Super::BeginPlay();
	OwnerCharacter = OwnerCharacter != nullptr ? OwnerCharacter : GetOwner() ? Cast<ABasePlayerCharacter>(GetOwner()) : nullptr;
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("NO OWNER CHARACTER AT BEGINPLAY"));
	}
	
}

void AWOGBaseMagic::OnMagicOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
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

	int32 MaxAmountMagics = MagicData.bIsAttacker ? 2 : 1;
	if (Equipment->MagicShortcutReferences.Num() < MaxAmountMagics)
	{
		FShortcutItemReference MagicRef;
		int32 KeyInt = Equipment->MagicShortcutReferences.Num() + 1;
		MagicRef.Key = *FString::FromInt(KeyInt);
		MagicRef.ItemShortcut = this;

		if (Equipment->SaveMagicShortcutReference(MagicRef))
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
				UE_LOG(LogTemp, Warning, TEXT("OWNER CHARACTER SET"));
				OwnerCharacter->Server_EquipMagic(MagicRef.Key, this);
				AttachToActor(OwnerCharacter, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			}
			return;
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Too many magics in the inventory"));
	}
}

void AWOGBaseMagic::OnMagicPickedUp(UAGR_InventoryManager* Inventory)
{
}

void AWOGBaseMagic::OnMagicEquip(AActor* User, FName SlotName)
{
	Multicast_OnMagicEquip(User, SlotName);

	if (!User) return;
	UAGRAnimMasterComponent* AnimMaster = UAGRLibrary::GetAnimationMaster(User);
	if (!AnimMaster) return;

	if (SlotName == NAME_MagicSlot_MagicPrimary)
	{
		AnimMaster->SetupBasePose(MagicData.MagicPoseTag);

		bool Success = GrantMagicAbilities(User);
		UE_LOG(LogTemp, Display, TEXT("WeaponGrantedAbilities applied: %d"), Success);
		SpawnIdleClass();
	}
	else
	{
		AnimMaster->SetupBasePose(TAG_Pose_Relax);

		bool Success = RemoveGrantedAbilities(User);
		UE_LOG(LogTemp, Display, TEXT("WeaponAbilities removed: %d"), Success);

		if (IdleActor)
		{
			IdleActor->Destroy();
		}
	}
}

void AWOGBaseMagic::OnMagicUnequip(AActor* User, FName SlotName)
{
}

void AWOGBaseMagic::Multicast_OnMagicEquip_Implementation(AActor* User, FName SlotName)
{
	if (SlotName == NAME_MagicSlot_MagicPrimary)
	{
		AttachToHands();
	}
	else
	{
		AttachToBack();
	}
}

void AWOGBaseMagic::AttachToHands()
{
	//TO-DO: 
	//Activate idle Effects on hands
	//Attach to OwnerCharacter
}

void AWOGBaseMagic::AttachToBack()
{
	//TO-DO: 
	//Deactivate idle Effects on hands
}

bool AWOGBaseMagic::GrantMagicAbilities(AActor* User)
{
	if (!HasAuthority() || MagicData.AbilitiesToGrant.IsEmpty()) return false;
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(User);
	if (!ASC) return false;
	for (auto Ability : MagicData.AbilitiesToGrant)
	{
		if (!Ability) continue;
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(Ability, 1, static_cast<int32>(Ability.GetDefaultObject()->AbilityInputID), User);
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

void AWOGBaseMagic::SetOwnerCharacter(ABasePlayerCharacter* NewOwner)
{
	if (NewOwner)
	{
		OwnerCharacter = NewOwner;
	}
	UE_LOG(LogTemp, Warning, TEXT("New owner of magic %s is : %s"), *GetNameSafe(this), *GetNameSafe(OwnerCharacter));
}

