// Fill out your copyright notice in the Description page of Project Settings.

#include "ActorComponents/WOGEnemyOrderComponent.h"
#include "Net/UnrealNetwork.h"
#include "PlayerCharacter/WOGAttacker.h"
#include "AI/Combat/WOGBaseSquad.h"
#include "WOG.h"

UWOGEnemyOrderComponent::UWOGEnemyOrderComponent()
{
	SetIsReplicated(true);

}

void UWOGEnemyOrderComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UWOGEnemyOrderComponent, CurrentSquads);
	DOREPLIFETIME(UWOGEnemyOrderComponent, OwnerAttacker);
}

void UWOGEnemyOrderComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerAttacker = Cast<AWOGAttacker>(GetOwner());
}

void UWOGEnemyOrderComponent::SendOrder(AWOGBaseSquad* Squad, const EEnemyOrder& NewOrder, const FTransform& TargetTansform, AActor* TargetActor)
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;
	if (!Squad)
	{
		UE_LOG(WOGLogSpawn, Error, TEXT("Invalid squand for order"));
		return;
	}

	Squad->SetCurrentSquadOrder(NewOrder);

	switch (NewOrder)
	{
	case EEnemyOrder::EEO_Hold:

		Squad->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		Squad->SetActorTransform(TargetTansform);
		break;

	case EEnemyOrder::EEO_Follow:
		if (GetNextAvailableSquadSlot())
		{
			Squad->AttachToComponent(GetNextAvailableSquadSlot(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		}
		break;

	case EEnemyOrder::EEO_AttackTarget:
		break;
	case EEnemyOrder::EEO_AttackRandom:
		break;
	default:
		break;
	}
}

USceneComponent* UWOGEnemyOrderComponent::GetNextAvailableSquadSlot() const
{
	if (GetOwner() && OwnerAttacker == nullptr)
	{
		Cast<AWOGAttacker>(GetOwner());
	}

	//No current active squads
	if (CurrentSquads.Num() == 0)
	{
		return OwnerAttacker->SquadSlot_0;
	}
	// One current active squad
	else if (CurrentSquads.Num() == 1)
	{
		return OwnerAttacker->SquadSlot_1;
	}
	//handles edge cases
	else
	{
		return nullptr;
	}
}

void UWOGEnemyOrderComponent::HandleCurrentSquads(AWOGBaseSquad* Squad, bool bAdd)
{
	if(!Squad) return;
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;

	if (bAdd)
	{
		//Add to the current squad array
		CurrentSquads.AddUnique(Squad);
	}
	else
	{
		//Remove from the CurrentSquad array
		if (CurrentSquads.IsEmpty()) return;

		for (int32 i = CurrentSquads.Num() - 1; i >= 0; i--)
		{
			if (CurrentSquads[i] == Squad)
			{
				CurrentSquads.Remove(Squad);
			}
		}
	}

}

void UWOGEnemyOrderComponent::Server_SendOrder_Implementation(AWOGBaseSquad* Squad, const EEnemyOrder& NewOrder, const FTransform& TargetTansform, AActor* TargetActor)
{
	SendOrder(Squad, NewOrder, TargetTansform, TargetActor);
}
