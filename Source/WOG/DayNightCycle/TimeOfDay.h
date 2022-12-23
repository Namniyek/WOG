// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TimeOfDay.generated.h"

UCLASS()
class WOG_API ATimeOfDay : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATimeOfDay();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Day/Night cycle")
	int32 RepCurrentTime = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Day/Night cycle")
	int32 StartingTime;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Day/Night cycle")
	float UpdateFrequency;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/*UFUNCTION()
	void OnRep_CurrentTime();*/

	void UpdateTime();

	float CurrentTime;

private:
	void ConvertTimeFormat(int32 MinutesToConvert);

	int32 CurrentHour = 0;
	int32 CurrentMinute = 0;


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintImplementableEvent)
	void TimeUpdated(int32 SyncedCurrentTime, int32 Hours, int32 Minutes);

	UFUNCTION(NetMulticast, unreliable)
	void Multicast_SyncCurrentTime(int32 TimeToSync);

};
