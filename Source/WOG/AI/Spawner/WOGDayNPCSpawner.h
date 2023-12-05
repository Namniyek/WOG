// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/SpawnInterface.h"
#include "WOGDayNPCSpawner.generated.h"



UCLASS()
class WOG_API AWOGDayNPCSpawner : public AActor, public ISpawnInterface
{
	GENERATED_BODY()
	
public:	
	AWOGDayNPCSpawner();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;



protected:
	virtual void BeginPlay() override;

	#pragma region TimeOfDay
	UFUNCTION()
	void TimeOfDayChanged(ETimeOfDay TOD);

	//TO-DO Test if needed, remove if not
	UFUNCTION()
	void KeyTimeHit(int32 CurrentTime);

	//TO-DO Test if needed, remove if not
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_CurrentTOD)
	ETimeOfDay CurrentTOD;

	//TO-DO Test if needed, remove if not
	UFUNCTION()
	void OnRep_CurrentTOD();

	virtual void HandleTODChange() {/*TO BE OVERRIDEN IN CHILDREN*/ };
	#pragma endregion

public:	

};
