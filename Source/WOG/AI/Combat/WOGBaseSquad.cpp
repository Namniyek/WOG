// Fill out your copyright notice in the Description page of Project Settings.
#include "AI/Combat/WOGBaseSquad.h"
#include "WOG.h"
#include "Net/UnrealNetwork.h"

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




