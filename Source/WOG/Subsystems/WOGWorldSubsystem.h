// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "DayNightCycle/TimeOfDay.h"
#include "GameplayTags.h"
#include "WOGWorldSubsystem.generated.h"


/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemUpgradedDelegate, FGameplayTag, ItemTag, int32, NewLevel);

UCLASS()
class WOG_API UWOGWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	
	UFUNCTION()
	void TimeOfDayChanged(ETimeOfDay TOD);

	UFUNCTION()
	void OnKeyTimeHit(int32 CurrentTime);

protected:
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Time of day")
	TObjectPtr<ATimeOfDay> TODActor;

public:
	UPROPERTY(BlueprintAssignable)
	FTimeOfDayChanged TimeOfDayChangedDelegate;

	UPROPERTY(BlueprintAssignable)
	FOnKeyTimeHit OnKeyTimeHitDelegate;

	ETimeOfDay CurrentTOD;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnItemUpgradedDelegate OnItemUpgradedDelegate;
};
