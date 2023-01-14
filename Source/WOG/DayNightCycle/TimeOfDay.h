// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WOG/Data/TODEnum.h"
#include "TimeOfDay.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTimeOfDayChanged, ETimeOfDay, TOD);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDayChanged, int32, DayNumber);

UCLASS()
class WOG_API ATimeOfDay : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATimeOfDay();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentTime, VisibleAnywhere, BlueprintReadOnly, Category = "Day/Night cycle")
	int32 RepCurrentTime = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Day/Night cycle")
	int32 StartingTime;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Day/Night cycle")
	float UpdateFrequency;

	UPROPERTY(BlueprintAssignable)
	FTimeOfDayChanged TimeOfDayChanged;

	UPROPERTY(BlueprintAssignable)
	FDayChanged DayChanged;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnRep_CurrentTime();

	void UpdateTime();

	float CurrentTime;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated)
	ETimeOfDay TimeOfDay;

private:
	void ConvertTimeFormat(int32 MinutesToConvert);

	int32 CurrentHour = 0;
	int32 CurrentMinute = 0;
	int32 CurrentDay = 1;

	void UpdateTimeOfDay(int32 Time);

	FTimerHandle UpdateTimeHandle;

public:	
	UFUNCTION(BlueprintImplementableEvent)
	void TimeUpdated(int32 SyncedCurrentTime, int32 Hours, int32 Minutes);

	FORCEINLINE ETimeOfDay GetTimeOfDay() { return TimeOfDay; }

	void StopCycle();

};
