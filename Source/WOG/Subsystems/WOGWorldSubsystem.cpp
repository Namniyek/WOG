// Fill out your copyright notice in the Description page of Project Settings.

#include "Subsystems/WOGWorldSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "WOG.h"

void UWOGWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogTemp, Warning, TEXT("WOG World subsystem initialized"));
}

void UWOGWorldSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UWOGWorldSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	TODActor = Cast<ATimeOfDay>(UGameplayStatics::GetActorOfClass(this, ATimeOfDay::StaticClass()));
	if (TODActor)
	{
		UE_LOG(WOGLogWorld, Warning, TEXT("TOD actor referenced"));
		TODActor->TimeOfDayChanged.AddDynamic(this, &ThisClass::TimeOfDayChanged);
		TODActor->OnKeyTimeHit.AddDynamic(this, &ThisClass::OnKeyTimeHit);
	}
	if (!TODActor)
	{
		UE_LOG(WOGLogWorld, Error, TEXT("TOD actor not referenced"));
	}
}

void UWOGWorldSubsystem::TimeOfDayChanged(ETimeOfDay TOD)
{
	CurrentTOD = TOD;
	TimeOfDayChangedDelegate.Broadcast(TOD);
	UE_LOG(WOGLogWorld, Display, TEXT("TimeOfDay = %s"), *UEnum::GetValueAsString(TOD));
}

void UWOGWorldSubsystem::OnKeyTimeHit(const int32 CurrentTime)
{
	OnKeyTimeHitDelegate.Broadcast(CurrentTime);
}
