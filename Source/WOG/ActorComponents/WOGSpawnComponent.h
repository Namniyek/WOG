// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WOGBaseActorComponent.h"
#include "Engine/DataTable.h"
#include "Types/CharacterTypes.h"
#include "Data/WOGDataTypes.h"
#include "WOGSpawnComponent.generated.h"

class UCameraComponent;
class AWOGBaseSquad;

/**
 * 
 */
 /** Struct used to define the types of buildables available */
USTRUCT(BlueprintType)
struct FSpawnables : public FTableRowBase
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Name"))
	FText Name = FText();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Icon"))
	TObjectPtr<UTexture2D> Icon = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Mesh", MakeStructureDefaultValue = "None"))
	TObjectPtr<UStaticMesh> Mesh = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Actor", MakeStructureDefaultValue = "None"))
	TObjectPtr<UClass> Actor = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Health", MakeStructureDefaultValue = "0.000000"))
	double MaxHealth = 0.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Amount Units", MakeStructureDefaultValue = "1"))
	int32 AmountUnits = 0;

	/**Capsule half height*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "HeightOffset", MakeStructureDefaultValue = "1"))
	float HeightOffset = 0.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Initial Setup")
	FCharacterAbilityData DefaultAbilitiesAndEffects;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Initial Setup")
	EEnemySquadType SquadType = EEnemySquadType::EEST_Melee;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Combat")
	float BaseDamage = 10.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Combat")
	TObjectPtr<UAnimMontage> AttackMontage = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Combat")
	float AttackRange = 150.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Combat")
	float DefendRange = 400.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Combat")
	TSubclassOf<UGameplayEffect> DamageEffect = nullptr;
};

UCLASS(meta = (BlueprintSpawnableComponent))
class WOG_API UWOGSpawnComponent : public UWOGBaseActorComponent
{
	GENERATED_BODY()

public:
	friend class AWOGAttacker;
	UWOGSpawnComponent();

private:
	FTimerHandle SpawnTimerHandle;
	TObjectPtr<AWOGAttacker> AttackerCharacter;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Building Setting")
	TObjectPtr<UDataTable> SpawnablesDataTable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Building Setting")
	TObjectPtr<UMaterialInstance> AllowedGhostMaterial;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Building Setting")
	TObjectPtr<UMaterialInstance> ForbiddenGhostMaterial;

	TArray<FSpawnables*> Spawnables;

public:

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	int32 LastIndexDataTable = 0;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default")
	bool bIsSpawnModeOn;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default")
	bool bCanSpawn;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Default")
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
	void ChangeMesh();

	UFUNCTION(Server, reliable)
	void Server_Spawn(FTransform Transform, int32 ID);
	void Spawn(FTransform Transform, int32 ID);

	TArray<FVector> GetSpawnLocations(const FVector& MiddleLocation, float GridSize, int32 Amount);
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AWOGBaseSquad> SquadClass;

public:

	UFUNCTION(BlueprintCallable)
	void LaunchSpawnMode();

	UFUNCTION(BlueprintCallable)
	void PlaceSpawn();

	UFUNCTION(BlueprintCallable)
	void HandleSpawnRotation(bool bRotateLeft);

};
