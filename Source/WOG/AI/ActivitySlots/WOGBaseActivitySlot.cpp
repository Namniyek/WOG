// Fill out your copyright notice in the Description page of Project Settings.
#include "AI/ActivitySlots/WOGBaseActivitySlot.h"
#include "Net/UnrealNetwork.h"
#include "WOG.h"
#include "Data/DataAssets/WOGAnimationCollectionData.h"

// Sets default values
AWOGBaseActivitySlot::AWOGBaseActivitySlot()
{
	PrimaryActorTick.bCanEverTick = false;

	bIsSlotReserved = false;

	ActivitySlotTag = FGameplayTag();
	MinMaxDuration.X = 10.f;
	MinMaxDuration.Y = 15.f;
	ActionMontage = nullptr;
	AnimationCollection = nullptr;
	bIsStatic = false;
}

void AWOGBaseActivitySlot::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWOGBaseActivitySlot, bIsSlotReserved);
}

void AWOGBaseActivitySlot::BeginPlay()
{
	Super::BeginPlay();

}

#if WITH_EDITOR
void AWOGBaseActivitySlot::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.GetPropertyName() == FName("ActivitySlotTag"))
	{
		UpdateActionMontage();
	}
}
#endif

void AWOGBaseActivitySlot::UpdateActionMontage()
{
	if (!AnimationCollection)
	{
		UE_LOG(WOGLogSpawn, Error, TEXT("Invalid AnimationCollection"));
		return;
	}

	if (!AnimationCollection->AnimationCollection.Contains(ActivitySlotTag))
	{
		UE_LOG(WOGLogSpawn, Error, TEXT("Tag not present in AnimationCollection"));
		return;
	}

	UAnimMontage* NewMontage = *AnimationCollection->AnimationCollection.Find(ActivitySlotTag);
	if (!NewMontage)
	{
		UE_LOG(WOGLogSpawn, Error, TEXT("Invalid Montage"));
		return;
	}

	ActionMontage = NewMontage;
}
	

