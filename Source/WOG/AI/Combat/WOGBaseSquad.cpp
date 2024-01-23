// Fill out your copyright notice in the Description page of Project Settings.
#include "AI/Combat/WOGBaseSquad.h"
#include "WOG.h"
#include "Net/UnrealNetwork.h"
#include "ActorComponents/WOGEnemyOrderComponent.h"
#include "Enemies/WOGBaseEnemy.h"
#include "Target/WOGBaseTarget.h"
#include "PlayerCharacter/WOGDefender.h"
#include "Kismet/GameplayStatics.h"
#include "Interfaces/TargetInterface.h"

AWOGBaseSquad::AWOGBaseSquad()
{
	PrimaryActorTick.bCanEverTick = false;
	SetActorTickEnabled(false);
	bReplicates = true;
	SetReplicateMovement(true);

	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComp"));
	SetRootComponent(RootComp);

	SquadType = EEnemySquadType::EEST_Melee;

	Slot_0 = CreateDefaultSubobject<USceneComponent>(TEXT("Slot_0"));
	Slot_0->SetupAttachment(GetRootComponent());
	Slot_0->bHiddenInGame = false;
	Slot_0->SetVisibility(true);
	SlotComponentsArray.AddUnique(Slot_0);

	Slot_1 = CreateDefaultSubobject<USceneComponent>(TEXT("Slot_1"));
	Slot_1->SetupAttachment(GetRootComponent());
	Slot_1->bHiddenInGame = false;
	Slot_1->SetVisibility(true);
	SlotComponentsArray.AddUnique(Slot_1);

	Slot_2 = CreateDefaultSubobject<USceneComponent>(TEXT("Slot_2"));
	Slot_2->SetupAttachment(GetRootComponent());
	Slot_2->bHiddenInGame = false;
	Slot_2->SetVisibility(true);
	SlotComponentsArray.AddUnique(Slot_2);

	Slot_3 = CreateDefaultSubobject<USceneComponent>(TEXT("Slot_3"));
	Slot_3->SetupAttachment(GetRootComponent());
	Slot_3->bHiddenInGame = false;
	Slot_3->SetVisibility(true);
	SlotComponentsArray.AddUnique(Slot_3);

	Slot_4 = CreateDefaultSubobject<USceneComponent>(TEXT("Slot_4"));
	Slot_4->SetupAttachment(GetRootComponent());
	Slot_4->bHiddenInGame = false;
	Slot_4->SetVisibility(true);
	SlotComponentsArray.AddUnique(Slot_4);

	Slot_5 = CreateDefaultSubobject<USceneComponent>(TEXT("Slot_5"));
	Slot_5->SetupAttachment(GetRootComponent());
	Slot_5->bHiddenInGame = false;
	Slot_5->SetVisibility(true);
	SlotComponentsArray.AddUnique(Slot_5);

	Slot_6 = CreateDefaultSubobject<USceneComponent>(TEXT("Slot_6"));
	Slot_6->SetupAttachment(GetRootComponent());
	Slot_6->bHiddenInGame = false;
	Slot_6->SetVisibility(true);
	SlotComponentsArray.AddUnique(Slot_6);

	Slot_7 = CreateDefaultSubobject<USceneComponent>(TEXT("Slot_7"));
	Slot_7->SetupAttachment(GetRootComponent());
	Slot_7->bHiddenInGame = false;
	Slot_7->SetVisibility(true);
	SlotComponentsArray.AddUnique(Slot_7);

	Slot_8 = CreateDefaultSubobject<USceneComponent>(TEXT("Slot_8"));
	Slot_8->SetupAttachment(GetRootComponent());
	Slot_8->bHiddenInGame = false;
	Slot_8->SetVisibility(true);
	SlotComponentsArray.AddUnique(Slot_8);
}

void AWOGBaseSquad::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWOGBaseSquad, CurrentSquadOrder);
	DOREPLIFETIME(AWOGBaseSquad, SquadSlots);
	DOREPLIFETIME(AWOGBaseSquad, CurrentTargetLocation);
	DOREPLIFETIME(AWOGBaseSquad, CurrentTargetActor);
	DOREPLIFETIME(AWOGBaseSquad, SquadType);
}

void AWOGBaseSquad::BeginPlay()
{
	Super::BeginPlay();
}

void AWOGBaseSquad::SendOrder(const EEnemyOrder& NewOrder, const FTransform& TargetTansform, AActor* TargetActor)
{
	/*
	*
	*TO-DO REFACTOR into smaller functions!!!!!!!!!!!!!!
	*This function is a monster
	*	
	*/

	if (!GetOwner() || !GetOwner()->HasAuthority()) return;

	UWOGEnemyOrderComponent* OrderComp = GetOwner()->GetComponentByClass<UWOGEnemyOrderComponent>();

	switch (NewOrder)
	{
	case EEnemyOrder::EEO_Hold:

		//Check if the Current target actor is valid and implements the Target Interface
		// if so, free the squad slot on the previous target
		if (CurrentTargetActor && CurrentTargetActor->GetClass()->ImplementsInterface(UTargetInterface::StaticClass()))
		{
			ITargetInterface::Execute_FreeCurrentMeleeSquadSlot(CurrentTargetActor);
		}

		/*
		*Squad should hold at the established position
		*/

		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		SetActorTransform(TargetTansform);
		SetEnemyStateOnSquad(EEnemyState::EES_AtSquadSlot);
		SetCurrentTargetActor(nullptr);

		SetCurrentSquadOrder(NewOrder);
		break;

	case EEnemyOrder::EEO_Follow:
		//Check if the Current target actor is valid and implements the Target Interface
		// if so, free the squad slot on the previous target
		if (CurrentTargetActor && CurrentTargetActor->GetClass()->ImplementsInterface(UTargetInterface::StaticClass()))
		{
			ITargetInterface::Execute_FreeCurrentMeleeSquadSlot(CurrentTargetActor);
		}

		/*
		*Squad should follow the owner around
		*/
		if(OrderComp && OrderComp->GetNextAvailableSquadSlot(this))
		{
			AttachToComponent(OrderComp->GetNextAvailableSquadSlot(this), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			SetEnemyStateOnSquad(EEnemyState::EES_AtSquadSlot);
			SetCurrentTargetActor(nullptr);

			SetCurrentSquadOrder(NewOrder);
		}
		break;

	case EEnemyOrder::EEO_AttackTarget:

		/*
		*Squad will attack a specific target
		* 1st, check if TargetActor is valid, if not, default to Follow order
		*/
		if (!TargetActor)
		{
			SendOrder(EEnemyOrder::EEO_Follow);
			UE_LOG(WOGLogCombat, Error, TEXT("TargetActor invalid, default to FOLLOW order"));
			return;
		}

		//Attach the squad actor to the squad slot on attacker
		if (OrderComp && OrderComp->GetNextAvailableSquadSlot(this))
		{
			AttachToComponent(OrderComp->GetNextAvailableSquadSlot(this), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		}

		//Check if TargetActor is a player or a building
		// Target is a Building
		if (TargetActor->IsA<AWOGBaseTarget>() && TargetActor->GetClass()->ImplementsInterface(UTargetInterface::StaticClass()))
		{
			//Check if target has an availability for squad
			if (SquadType == EEnemySquadType::EEST_Melee && ITargetInterface::Execute_IsCurrentMeleeSquadSlotAvailable(TargetActor))
			{
				ITargetInterface::Execute_SetCurrentMeleeSquadSlot(TargetActor, this);
				SetCurrentTargetActor(TargetActor);
				SetEnemyStateOnSquad(EEnemyState::EES_AtTargetSlot);
				SetCurrentSquadOrder(NewOrder);
				return;
			}

			if (SquadType == EEnemySquadType::EEST_Ranged && ITargetInterface::Execute_IsCurrentRangedSquadSlotAvailable(TargetActor))
			{
				ITargetInterface::Execute_SetCurrentRangedSquadSlot(TargetActor, this);
				SetCurrentTargetActor(TargetActor);
				SetEnemyStateOnSquad(EEnemyState::EES_AtTargetSlot);
				SetCurrentSquadOrder(NewOrder);
				return;
			}

			// if not, default to Follow order
			SendOrder(EEnemyOrder::EEO_Follow);
			UE_LOG(WOGLogCombat, Error, TEXT("Too many squads attaching same target"));
		}

		//Target is a player
		if (TargetActor->IsA<AWOGDefender>() && TargetActor->GetClass()->ImplementsInterface(UTargetInterface::StaticClass()))
		{
			//Check if target has an availability for squad
			if (SquadType == EEnemySquadType::EEST_Melee && ITargetInterface::Execute_IsCurrentMeleeSquadSlotAvailable(TargetActor))
			{
				ITargetInterface::Execute_SetCurrentMeleeSquadSlot(TargetActor, this);
				SetCurrentTargetActor(TargetActor);
				SetEnemyStateOnSquad(EEnemyState::EES_AtTargetPlayer);
				SetCurrentSquadOrder(NewOrder);
				return;
			}

			if (SquadType == EEnemySquadType::EEST_Ranged && ITargetInterface::Execute_IsCurrentRangedSquadSlotAvailable(TargetActor))
			{
				ITargetInterface::Execute_SetCurrentRangedSquadSlot(TargetActor, this);
				SetCurrentTargetActor(TargetActor);
				SetEnemyStateOnSquad(EEnemyState::EES_AtTargetPlayer);
				SetCurrentSquadOrder(NewOrder);
				return;
			}

			// if not, default to Follow order
			SendOrder(EEnemyOrder::EEO_Follow);
			UE_LOG(WOGLogCombat, Error, TEXT("Too many squads attaching same target"));
		}

		break;
	case EEnemyOrder::EEO_AttackRandom:

		/*
		*Squad will attack a random target within range
		*/
		TargetActor = FindRandomTarget();
		if (TargetActor)
		{
			SendOrder(EEnemyOrder::EEO_AttackTarget, FTransform(), TargetActor);
			return;
		}

		// if not, default to Follow order
		SendOrder(EEnemyOrder::EEO_Follow);
		UE_LOG(WOGLogCombat, Error, TEXT("No valid random target"));
		break;
	default:
		break;
	}
}

void AWOGBaseSquad::OnCurrentTargetDestroyed(AActor* Destroyer)
{
	SendOrder(EEnemyOrder::EEO_Follow);
}

AActor* AWOGBaseSquad::FindRandomTarget()
{
	TArray<AActor*> OutActors;
	UGameplayStatics::GetAllActorsWithInterface(GetWorld(), UTargetInterface::StaticClass(), OutActors);

	if (OutActors.IsEmpty())
	{
		UE_LOG(WOGLogSpawn, Error, TEXT("OutActors is EMPTY"));
		return nullptr;
	}

	TArray<AActor*> ReturnActors = {};

	for (auto Actor : OutActors)
	{
		UE_LOG(WOGLogSpawn, Display, TEXT("%s iterated"), *GetNameSafe(Actor));

		if (!Actor->IsA<AWOGBaseTarget>())
		{
			UE_LOG(WOGLogSpawn, Warning, TEXT("%s is not a WOGBaseTarget"), *GetNameSafe(Actor));
			continue;
		}

		if (SquadType == EEnemySquadType::EEST_Melee && ITargetInterface::Execute_IsCurrentMeleeSquadSlotAvailable(Actor))
		{
			ReturnActors.AddUnique(Actor);
			UE_LOG(WOGLogSpawn, Display, TEXT("%s added to ReturnActors"), *GetNameSafe(Actor));
			continue;
		}

		if (SquadType == EEnemySquadType::EEST_Ranged && ITargetInterface::Execute_IsCurrentRangedSquadSlotAvailable(Actor))
		{
			ReturnActors.AddUnique(Actor);
			UE_LOG(WOGLogSpawn, Display, TEXT("%s added to ReturnActors"), *GetNameSafe(Actor));
			continue;
		}
	}

	if (ReturnActors.IsEmpty())
	{
		UE_LOG(WOGLogSpawn, Error, TEXT("ReturnActors is EMPTY"));
		return nullptr;
	}

	AActor* ClosestTarget = GetClosestActor(ReturnActors);
	return ClosestTarget;
}

AActor* AWOGBaseSquad::GetClosestActor(TArray<AActor*> InArray)
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

void AWOGBaseSquad::SetEnemyStateOnSquad(const EEnemyState& NewState)
{
	for (FEnemyCombatSlot EnemySlot : SquadSlots)
	{
		if (EnemySlot.CurrentEnemy)
		{
			EnemySlot.CurrentEnemy->SetCurrentEnemyState(NewState);
		}
	}
}

void AWOGBaseSquad::SetCurrentSquadOrder(const EEnemyOrder& NewOrder)
{
	if (!HasAuthority()) return;

	CurrentSquadOrder = NewOrder;
}

void AWOGBaseSquad::SetCurrentTargetActor(AActor* NewTarget)
{
	if (!HasAuthority()) return;

	if (NewTarget != nullptr)
	{
		CurrentTargetActor = NewTarget;
		AWOGBaseTarget* Target = Cast<AWOGBaseTarget>(CurrentTargetActor);
		if (Target)
		{
			Target->OnTargetDestroyedDelegate.AddDynamic(this, &ThisClass::OnCurrentTargetDestroyed);
		}

		return;
	}

	if (NewTarget == nullptr && CurrentTargetActor != nullptr)
	{
		AWOGBaseTarget* Target = Cast<AWOGBaseTarget>(CurrentTargetActor);
		if (Target)
		{
			Target->OnTargetDestroyedDelegate.Clear();
		}

		CurrentTargetActor = NewTarget;
		return;
	}







}

void AWOGBaseSquad::SetCurrentTargetLocation(const FVector_NetQuantize& NewTarget)
{
	if (!HasAuthority()) return;
	CurrentTargetLocation = NewTarget;
}

void AWOGBaseSquad::DeregisterDeadSquadMember(AWOGBaseEnemy* DeadEnemy)
{
	if (!HasAuthority()) return;
	for (int32 i = 0; i < SquadSlots.Num(); i++)
	{
		if (SquadSlots[i].CurrentEnemy && SquadSlots[i].CurrentEnemy == DeadEnemy)
		{
			UE_LOG(WOGLogCombat, Display, TEXT("%s is dead and deregistered from squad"), *GetNameSafe(SquadSlots[i].CurrentEnemy));
			SquadSlots[i].CurrentEnemy = nullptr;
			CheckIsSquadEmpty();
		}
	}
}

void AWOGBaseSquad::CheckIsSquadEmpty()
{
	UE_LOG(WOGLogCombat, Display, TEXT("Checking if squad is empty..."));

	for (int32 i = 0; i < SquadSlots.Num(); i++)
	{
		if (SquadSlots[i].CurrentEnemy != nullptr)
		{
			//we parsed through the whole squad and found a valid pointer to some enemy
			//Squad NOT empty
			UE_LOG(WOGLogCombat, Display, TEXT("%s is still in the squad"), *GetNameSafe(SquadSlots[i].CurrentEnemy));
			return;
		}
	}

	//we parsed through the whole squad and found no valid pointers to anybody
	//Squad is empty
	UE_LOG(WOGLogCombat, Display, TEXT("Squad is empty"));
	DeregisterSquad();
	return;
}

void AWOGBaseSquad::DeregisterSquad()
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;
	UE_LOG(WOGLogCombat, Display, TEXT("Attempting to deregister squad..."));

	if (CurrentTargetActor && CurrentTargetActor->GetClass()->ImplementsInterface(UTargetInterface::StaticClass()))
	{
		ITargetInterface::Execute_FreeCurrentMeleeSquadSlot(CurrentTargetActor);
		ITargetInterface::Execute_FreeCurrentRangedSquadSlot(CurrentTargetActor);
	}

	UWOGEnemyOrderComponent* OrderComp = GetOwner()->GetComponentByClass<UWOGEnemyOrderComponent>();
	if (OrderComp)
	{
		OrderComp->HandleCurrentSquads(this, false);
		UE_LOG(WOGLogCombat, Display, TEXT("HandleCurrentSquad() called"));
	}

	Destroy();
}


