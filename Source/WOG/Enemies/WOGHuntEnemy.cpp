// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemies/WOGHuntEnemy.h"
#include "WOG.h"
#include "Net/UnrealNetwork.h"
#include "Engine/DataTable.h"
#include "Types/CharacterTypes.h"
#include "AbilitySystemComponent.h"
#include "ActorComponents/WOGAbilitySystemComponent.h"
#include "Components/SphereComponent.h"
#include "PlayerCharacter/BasePlayerCharacter.h"
#include "Libraries/WOGBlueprintLibrary.h"
#include "AbilitySystem/AttributeSets/WOGAttributeSetBase.h"
#include "ActorComponents/WOGUIManagerComponent.h"

AWOGHuntEnemy::AWOGHuntEnemy()
{
	MinionNames = {};
	CharacterData.bIsAttacker = false;

	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	AgroSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AgroSphere"));
	AgroSphere->SetupAttachment(GetRootComponent());
	AgroSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	AgroSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	AgroSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	AgroSphere->InitSphereRadius(5000.f);
	AgroSphere->SetGenerateOverlapEvents(true);
}

void AWOGHuntEnemy::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void AWOGHuntEnemy::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWOGHuntEnemy, MinionLevel);
	DOREPLIFETIME(AWOGHuntEnemy, CurrentTarget);
	DOREPLIFETIME(AWOGHuntEnemy, CurrentTargetArray);
}

void AWOGHuntEnemy::BeginPlay()
{
	Super::BeginPlay();

	MinionLevel = 0;

	InitData();

	if (AgroSphere && HasAuthority())
	{
		AgroSphere->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnAgroOverlap);
		AgroSphere->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnAgroEndOverlap);
		UE_LOG(WOGLogSpawn, Display, TEXT("AgroSphere Callbacks bound"));
	}

	if (AbilitySystemComponent)
	{
		AttributeSet = const_cast<UWOGAttributeSetBase*>(Cast<UWOGAttributeSetBase>(AbilitySystemComponent->GetOrCreateWOGAttributeSubobject(UWOGAttributeSetBase::StaticClass())));
	}

	if (AbilitySystemComponent && AttributeSet && HasAuthority())
	{
		ApplyDefaultEffects();
		GiveDefaultAbilities();
	}

	CurrentEnemyState = EEnemyState::EES_Idle;

	OnAttributeChangedDelegate.AddDynamic(this, &ThisClass::OnAttributeChangedCallback);
}

void AWOGHuntEnemy::Destroyed()
{
	
	
	Super::Destroyed();
}

void AWOGHuntEnemy::Elim(bool bPlayerLeftGame)
{
	InjectVendorWithSpawnItem();
	Super::Elim(bPlayerLeftGame);
}

AActor* AWOGHuntEnemy::GetSquadCurrentTargetActor_Implementation()
{
	return CurrentTarget;
}

void AWOGHuntEnemy::SetCurrentTarget_Implementation(AActor* NewTarget)
{
	if (!HasAuthority()) return;
	CurrentTarget = NewTarget;
}

void AWOGHuntEnemy::InitData()
{
	if (!HasAuthority()) return;

	if (!MinionDataTable)
	{
		UE_LOG(WOGLogSpawn, Error, TEXT("Invalid Spawn DataTable"));
		return;
	}

	if (MinionLevel >= MinionNames.Num())
	{
		return;
	}

	TArray<FName> SpawnNamesArray = MinionDataTable->GetRowNames();
	FSpawnables* SpawnDataRow = nullptr;

	for (auto SpawnRowName : SpawnNamesArray)
	{
		if (SpawnRowName == MinionNames[MinionLevel])
		{
			SpawnDataRow = MinionDataTable->FindRow<FSpawnables>(MinionNames[MinionLevel], TEXT(""));
			break;
		}
	}
	if (SpawnDataRow)
	{
		SetDefaultAbilitiesAndEffects(SpawnDataRow->DefaultAbilitiesAndEffects);
		SetBaseDamage(SpawnDataRow->BaseDamage);
		SetAttackRange(SpawnDataRow->AttackRange);
		SetDefendRange(SpawnDataRow->DefendRange);
		SetDamageEffect(SpawnDataRow->DamageEffect);
		SetSecondaryDamageEffect(SpawnDataRow->SecondaryDamageEffect);
		SetCosmeticsDataAsset(SpawnDataRow->CosmeticsDataAsset);
		SetCharacterData(SpawnDataRow->CharacterData);
	}
	else
	{
		UE_LOG(WOGLogSpawn, Error, TEXT("Invalid SpawnDataRow"));
	}
}

AActor* AWOGHuntEnemy::FindClosestPlayerTarget_Implementation()
{
	CurrentTarget = FindRandomClosestPlayer();

	if (CurrentTarget == nullptr)
	{
		SetCurrentEnemyState(EEnemyState::EES_Idle);
	}

	return CurrentTarget;
}

AActor* AWOGHuntEnemy::GetIdleSpotLocation_Implementation()
{
	return IdleSpotLocation;
}

AActor* AWOGHuntEnemy::FindRandomClosestPlayer()
{
	if (CurrentTargetArray.IsEmpty())
	{
		UE_LOG(WOGLogSpawn, Error, TEXT("CurrentTargetArray is EMPTY"));
		return nullptr;
	}

	TArray<AActor*> ReturnActors = {};

	for (auto Actor : CurrentTargetArray)
	{
		UE_LOG(WOGLogSpawn, Display, TEXT("%s iterated"), *GetNameSafe(Actor));

		if (!Actor->IsA<ABasePlayerCharacter>() || GetCharacterData().bIsAttacker == UWOGBlueprintLibrary::GetCharacterData(Actor).bIsAttacker)
		{
			UE_LOG(WOGLogSpawn, Warning, TEXT("%s is not a valid target"), *GetNameSafe(Actor));
			continue;
		}

		ReturnActors.AddUnique(Actor);
	}

	if (ReturnActors.IsEmpty())
	{
		UE_LOG(WOGLogSpawn, Error, TEXT("ReturnActors is EMPTY"));
		return nullptr;
	}

	AActor* ClosestTarget = GetClosestActor(ReturnActors);
	return ClosestTarget;
}

AActor* AWOGHuntEnemy::GetClosestActor(TArray<AActor*> InArray)
{
	AActor* ClosestActor = nullptr;
	float ClosestDistance = 1000000000.f;

	for (auto Actor : InArray)
	{
		float CurrentDistance = Actor->GetDistanceTo(this);
		ClosestDistance = CurrentDistance <= ClosestDistance ? CurrentDistance : ClosestDistance;
		ClosestActor = CurrentDistance <= ClosestDistance ? Actor : ClosestActor;
	}

	return ClosestActor;
}

void AWOGHuntEnemy::InjectVendorWithSpawnItem() const
{
	if(!HasAuthority()) return;
	if(!AssignedVendor) return;

	FText OutNote;
	UAGRLibrary::GetInventory(static_cast<const AActor*>(AssignedVendor))->AddItemsOfClass(SpawnItemForVendor, 1, OutNote);
}

void AWOGHuntEnemy::PlayerCharacterEndedOverlap(AWOGBaseCharacter* LeavingPlayer)
{
	if (!HasAuthority()) return;
	if (!LeavingPlayer) return;

	if (LeavingPlayer == CurrentTarget)
	{
		FindNewTarget();
	}
	
	if (CurrentTargetArray.Contains(LeavingPlayer))
	{
		int32 Amount = CurrentTargetArray.Remove(LeavingPlayer);
		OnPlayerExitedAgroSphere(LeavingPlayer);
		
		if (LeavingPlayer->GetController())
		{
			UWOGUIManagerComponent* UIManager = UWOGBlueprintLibrary::GetUIManagerComponent(LeavingPlayer->GetController());
			if (UIManager)
			{
				UIManager->Client_RemoveHuntWidget();
				UE_LOG(WOGLogUI, Display, TEXT("HuntWidget removed"));
			}
		}
	}

	if (CurrentTargetArray.IsEmpty())
	{
		CurrentTarget = nullptr;
		SetCurrentEnemyState(EEnemyState::EES_Idle);
		UE_LOG(WOGLogSpawn, Display, TEXT("Target array empty. No CurrentTarget"));
	}
}

void AWOGHuntEnemy::OnAgroOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority()) return;
	if (!OtherActor) return;

	if (!CurrentTargetArray.Contains(OtherActor) && OtherActor->IsA<ABasePlayerCharacter>() && GetCharacterData().bIsAttacker != UWOGBlueprintLibrary::GetCharacterData(OtherActor).bIsAttacker)
	{
		CurrentTargetArray.AddUnique(OtherActor);
		OnPlayerEnteredAgroSphere(OtherActor);

		AWOGBaseCharacter* Pawn = Cast<AWOGBaseCharacter>(OtherActor);
		if (Pawn && Pawn->GetController())
		{
			Pawn->OnCharacterDeadDelegate.AddDynamic(this, &ThisClass::PlayerCharacterEndedOverlap);
			
			UWOGUIManagerComponent* UIManager = UWOGBlueprintLibrary::GetUIManagerComponent(Pawn->GetController());
			if (UIManager)
			{
				UIManager->Client_AddHuntWidget(this);
				UE_LOG(WOGLogUI, Display, TEXT("HuntWidget added"));
			}
		}
	}

	if (OtherActor->IsA<ABasePlayerCharacter>() && CurrentTarget == nullptr)
	{
		CurrentTarget = OtherActor;
		SetCurrentEnemyState(EEnemyState::EES_AtTargetPlayer);
	}
}

void AWOGHuntEnemy::OnAgroEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if(!HasAuthority()) return;
	
	AWOGBaseCharacter* LeavingPlayer = Cast<AWOGBaseCharacter>(OtherActor);
	PlayerCharacterEndedOverlap(LeavingPlayer);
}

void AWOGHuntEnemy::OnAttributeChangedCallback(FGameplayAttribute ChangedAttribute, float NewValue, float MaxValue)
{
	if (!HasAuthority()) return;
	if (MinionLevel != 0) return;
	if (ChangedAttribute == GetAttributeSetBase()->GetHealthAttribute() && (NewValue/MaxValue <= 0.5f))
	{
		SetCurrentEnemyState(EEnemyState::EES_LevelUp);
	}
}

void AWOGHuntEnemy::OnRep_MinionLevel(const int32& OldLevel)
{
	GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Green, (FString("HuntEnemy leveled up on:") + UEnum::GetValueAsString(GetLocalRole())));
}

void AWOGHuntEnemy::SetMinionLevel(const int32& NewLevel)
{
	if (!HasAuthority()) return;

	MinionLevel = NewLevel > MinionLevel ? NewLevel : MinionLevel;
	ClearAbilities();
	InitData();
	MergeAttackTagMaps();
	GiveDefaultAbilities();
	
	//Updates current target and enemy state
	CurrentTarget = FindRandomClosestPlayer();
	if (CurrentTarget)
	{
		SetCurrentEnemyState(EEnemyState::EES_AtTargetPlayer);
	}
	else
	{
		SetCurrentEnemyState(EEnemyState::EES_Idle);
	}
}

void AWOGHuntEnemy::ClearAbilities()
{
	if (!HasAuthority() || !AbilitySystemComponent) return;

	for (auto Spec : AbilitySystemComponent->GetActivatableAbilities())
	{
		if (Spec.Handle.IsValid())
		{
			AbilitySystemComponent->ClearAbility(Spec.Handle);
		}
	}
}

void AWOGHuntEnemy::FindNewTarget()
{
	if (!HasAuthority()) return;

	if (CurrentTargetArray.IsEmpty())
	{
		CurrentTarget = nullptr;
		SetCurrentEnemyState(EEnemyState::EES_Idle);
		return;
	}

	for (AActor* Target : CurrentTargetArray)
	{
		if (Target && Target != CurrentTarget)
		{
			CurrentTarget = Target;
			SetCurrentEnemyState(EEnemyState::EES_AtTargetPlayer);
			UE_LOG(WOGLogSpawn, Display, TEXT("CurrentTarget changed. New CurrentTarget is %s"), *GetNameSafe(Target));
			return;
		}
	}

	//No matching target found
	CurrentTarget = nullptr;
	SetCurrentEnemyState(EEnemyState::EES_Idle);
}

void AWOGHuntEnemy::MergeAttackTagMaps()
{
	if (!AdditionalCloseAttackTagsMap.IsEmpty())
	{
		//CloseAttackTagsMap.Empty();
		CloseAttackTagsMap.Append(AdditionalCloseAttackTagsMap);
	}
	if (!AdditionalMidAttackTagsMap.IsEmpty())
	{
		AttackTagsMap.Append(AdditionalMidAttackTagsMap);
	}
	if (!AdditionalRangedAttackTagsMap.IsEmpty())
	{
		RangedAttackTagsMap.Append(AdditionalRangedAttackTagsMap);
	}
}

