// Fill out your copyright notice in the Description page of Project Settings.
#include "AI/Combat/WOGBaseSquad.h"
#include "WOG.h"
#include "Net/UnrealNetwork.h"
#include "ActorComponents/WOGEnemyOrderComponent.h"
#include "Enemies/WOGBaseEnemy.h"

AWOGBaseSquad::AWOGBaseSquad()
{
	PrimaryActorTick.bCanEverTick = false;
	SetActorTickEnabled(false);
	bReplicates = true;
	SetReplicateMovement(true);

	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComp"));
	SetRootComponent(RootComp);

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
}

void AWOGBaseSquad::BeginPlay()
{
	Super::BeginPlay();

	/*UWOGEnemyOrderComponent* OrderComp = GetOwner()->GetComponentByClass<UWOGEnemyOrderComponent>();
	if (OrderComp)
	{
		OrderComp->SetCurrentlySelectedSquad(OrderComp->GetCurrentSquads().Last());
	}*/
}

void AWOGBaseSquad::SendOrder(const EEnemyOrder& NewOrder, const FTransform& TargetTansform, AActor* TargetActor)
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;

	UWOGEnemyOrderComponent* OrderComp = GetOwner()->GetComponentByClass<UWOGEnemyOrderComponent>();

	SetCurrentSquadOrder(NewOrder);

	switch (CurrentSquadOrder)
	{
	case EEnemyOrder::EEO_Hold:

		/*
		*Squad should hold at the established position
		*/
		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		SetActorTransform(TargetTansform);
		SetEnemyStateOnSquad(EEnemyState::EES_AtSquadSlot);
		break;

	case EEnemyOrder::EEO_Follow:

		/*
		*Squad should follow the owner around
		*/
		if(OrderComp && OrderComp->GetNextAvailableSquadSlot(this))
		{
			AttachToComponent(OrderComp->GetNextAvailableSquadSlot(this), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			SetEnemyStateOnSquad(EEnemyState::EES_AtSquadSlot);
		}
		break;

	case EEnemyOrder::EEO_AttackTarget:

		/*
		*Squad will attack a specific target
		*/
		if (OrderComp && OrderComp->GetNextAvailableSquadSlot(this))
		{
			AttachToComponent(OrderComp->GetNextAvailableSquadSlot(this), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		}

		if (TargetActor)
		{
			SetCurrentTargetActor(TargetActor);
			SetEnemyStateOnSquad(EEnemyState::EES_AtTargetSlot);
		}
		break;
	case EEnemyOrder::EEO_AttackRandom:

		/*
		*Squad will attack a random target within range
		*/
		if (OrderComp && OrderComp->GetNextAvailableSquadSlot(this))
		{
			AttachToComponent(OrderComp->GetNextAvailableSquadSlot(this), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		}

		if (TargetActor)
		{
			SetCurrentTargetActor(TargetActor);
			SetEnemyStateOnSquad(EEnemyState::EES_AtTargetSlot);
		}

		break;
	default:
		break;
	}
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

void AWOGBaseSquad::SetCurrentTargetActor(AActor*& NewTarget)
{
	if (!HasAuthority()) return;
	CurrentTargetActor = NewTarget;
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
	UWOGEnemyOrderComponent* OrderComp = GetOwner()->GetComponentByClass<UWOGEnemyOrderComponent>();
	if (OrderComp)
	{
		OrderComp->HandleCurrentSquads(this, false);
		UE_LOG(WOGLogCombat, Display, TEXT("HandleCurrentSquad() called"));
	}

	Destroy();
}


