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

	UPROPERTY(ReplicatedUsing = OnRep_CurrentTime, VisibleAnywhere)
	int32 CurrentTime = 0;
	int32 StartingTime = 1800;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnRep_CurrentTime();

	void UpdateTime();


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
