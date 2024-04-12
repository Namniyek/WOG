// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WOGBaseActorComponent.h"
#include "Types/CharacterTypes.h"
#include "WOGSpawnComponent.generated.h"

class UCameraComponent;
class AWOGBaseSquad;

UCLASS(meta = (BlueprintSpawnableComponent))
class WOG_API UWOGSpawnComponent : public UWOGBaseActorComponent
{
	GENERATED_BODY()

public:
	friend class AWOGAttacker;
	UWOGSpawnComponent();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


private:
	FTimerHandle SpawnTimerHandle;
	TObjectPtr<AWOGAttacker> AttackerCharacter;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Building Setting")
	TObjectPtr<UMaterialInstance> AllowedGhostMaterial;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Building Setting")
	TObjectPtr<UMaterialInstance> ForbiddenGhostMaterial;

	UPROPERTY(Replicated)
	TArray<FSpawnables> Spawnables;

public:

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	int32 LastIndexDataTable = 0;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default")
	bool bIsSpawnModeOn;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default")
	bool bCanSpawn;

	UPROPERTY(Replicated, BlueprintReadWrite, EditDefaultsOnly, Category = "Default")
	int32 SpawnID;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default")
	FTransform SpawnTransform;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default")
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default")
	TObjectPtr<UStaticMeshComponent> SpawnGhost;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default")
	TObjectPtr<AActor> CurrentHitActor;

protected: 
	virtual void BeginPlay() override;
	
	void StopSpawnMode();
	void SpawnDelay();
	void SpawnCycle();

	void SpawnSpawnGhost();
	void GiveSpawnColor(bool IsAllowed);

	bool CheckForOverlap();

	UFUNCTION(BlueprintCallable)
	void ChangeMesh(int32 ID);

	UFUNCTION(BlueprintCallable, Server, reliable)
	void Server_Spawn(FTransform Transform, int32 ID);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void Spawn(FTransform Transform, int32 ID);

	TArray<FVector> GetSpawnLocations(const FVector& MiddleLocation, float GridSize, int32 Amount);
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AWOGBaseSquad> SquadClass;

	UFUNCTION(Server, reliable)
	void Server_SetSpawnables(const TArray<FSpawnables>& InSpawnables);

	bool SetSpawnables();

	bool CheckCost();
	void DeductCost();

public:

	UFUNCTION(BlueprintCallable)
	void LaunchSpawnMode();

	UFUNCTION(BlueprintCallable)
	void PlaceSpawn() const;

	UFUNCTION(BlueprintCallable)
	void HandleSpawnRotation(bool bRotateLeft);

	UFUNCTION(BlueprintPure)
	FORCEINLINE TArray<FSpawnables> GetSpawnables() const { return Spawnables; }

};
