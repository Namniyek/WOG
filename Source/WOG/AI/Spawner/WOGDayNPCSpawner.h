// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/SpawnInterface.h"
#include "Data/WOGDataTypes.h"
#include "WOGDayNPCSpawner.generated.h"

class AWOGBaseDayNPC;

UCLASS()
class WOG_API AWOGDayNPCSpawner : public AActor, public ISpawnInterface
{
	GENERATED_BODY()
	
public:	
	AWOGDayNPCSpawner();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void SetSpawnerActive_Implementation(bool NewActive);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Setup")
	TArray<FVector3DWithWidget> SpawnLocations;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup")
	TMap<TSubclassOf<AWOGBaseDayNPC>, int32> SpawnMap;

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

	virtual void HandleTODChange();
	#pragma endregion

	#pragma region Spawn
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bIsSpawnerActive;

	virtual bool StartSpawn();
	virtual void SpawnNPC(const TSubclassOf<AActor>& ClassToSpawn, const FVector& Location);

	UFUNCTION()
	virtual void DelayedSpawnNPC(const TSubclassOf<AActor>& ClassToSpawn, const FVector& Location);

	#pragma endregion

private:
	#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	#endif

public:	

	void SetIsSpawnerActive(bool NewActive);

};
