// Fill out your copyright notice in the Description page of Project Settings.

#include "ActorComponents/WOGEnemyOrderComponent.h"
#include "Net/UnrealNetwork.h"
#include "PlayerCharacter/WOGAttacker.h"
#include "AI/Combat/WOGBaseSquad.h"
#include "WOG.h"

UWOGEnemyOrderComponent::UWOGEnemyOrderComponent()
{
	SetIsReplicated(true);

	MaxAmountSquads = 3;
}

void UWOGEnemyOrderComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UWOGEnemyOrderComponent, CurrentSquads);
	DOREPLIFETIME(UWOGEnemyOrderComponent, OwnerAttacker);
	DOREPLIFETIME(UWOGEnemyOrderComponent, CurrentlySelectedSquad);
}

void UWOGEnemyOrderComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerAttacker = Cast<AWOGAttacker>(GetOwner());
}

USceneComponent* UWOGEnemyOrderComponent::GetNextAvailableSquadSlot(AWOGBaseSquad* Squad) const
{
	if (GetOwner() && OwnerAttacker == nullptr)
	{
		Cast<AWOGAttacker>(GetOwner());
	}

	for(int32 i = 0; i < CurrentSquads.Num(); i++)
	{
		if (CurrentSquads[i] && Squad == CurrentSquads[i])
		{
			if (i == 0)
			{
				return OwnerAttacker->SquadSlot_0;
			}
			else if (i == 1)
			{
				return OwnerAttacker->SquadSlot_1;
			}
			else if (i == 2)
			{
				return OwnerAttacker->SquadSlot_2;
			}
		}
	}
	return nullptr;
}

void UWOGEnemyOrderComponent::HandleCurrentSquads(AWOGBaseSquad* Squad, bool bAdd)
{
	if (!Squad)
	{
		return;
	}
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

		IncreaseCurrentlySelectedSquad();
	}

}

void UWOGEnemyOrderComponent::SetCurrentlySelectedSquad(AWOGBaseSquad* NewSquad)
{
	CurrentlySelectedSquad = NewSquad;
	UE_LOG(WOGLogSpawn, Warning, TEXT("Local role of %s is %s for %s"), *GetNameSafe(GetOwner()), *UEnum::GetValueAsString(GetOwnerRole()), *GetNameSafe(NewSquad));
}

void UWOGEnemyOrderComponent::IncreaseCurrentlySelectedSquad()
{
	for (int32 i = 0; i < CurrentSquads.Num(); i++)
	{
		if (CurrentSquads[i] && CurrentSquads[i] == CurrentlySelectedSquad)
		{
			if (i == CurrentSquads.Num() - 1)
			{
				SetCurrentlySelectedSquad(CurrentSquads[0]);
			}
			else
			{
				SetCurrentlySelectedSquad(CurrentSquads[i+1]);
			}
			return;
		}
	}
}

void UWOGEnemyOrderComponent::DecreaseCurrentlySelectedSquad()
{
	for (int32 i = 0; i < CurrentSquads.Num(); i++)
	{
		if (CurrentSquads[i] && CurrentSquads[i] == CurrentlySelectedSquad)
		{
			if (i == 0)
			{
				SetCurrentlySelectedSquad(CurrentSquads[CurrentSquads.Num()-1]);
			}
			else
			{
				SetCurrentlySelectedSquad(CurrentSquads[i - 1]);
			}
			return;
		}
	}
}

void UWOGEnemyOrderComponent::Server_SendOrder_Implementation(AWOGBaseSquad* Squad, const EEnemyOrder& NewOrder, const FTransform& TargetTansform, AActor* TargetActor)
{
	Squad->SendOrder(NewOrder, TargetTansform, TargetActor);
}
