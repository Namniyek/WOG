// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "Interfaces/BuildingInterface.h"
#include "Interfaces/TargetInterface.h"
#include "AI/Combat/WOGBaseSquad.h"
#include "WOGBaseBuildable.generated.h"

class UWOGSpawnCosmetics;

/**
 * 
 */
UCLASS()
class WOG_API AWOGBaseBuildable : public AStaticMeshActor, public IBuildingInterface, public ITargetInterface
{
	GENERATED_BODY()

public:
	AWOGBaseBuildable();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnTargetDestroyedDelegate OnTargetDestroyedDelegate;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Build Properties")
	TMap<AActor*, int32> ChildrenBuilds;

protected:
	virtual void BeginPlay() override;
	virtual void BuildExtensions();
	virtual void DestroyChildren();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = "OnRep_BuildHealth", Category = "Build Properties")
	float BuildHealth;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Build Properties")
	float BuildMaxHealth;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Build Properties")
	float BuildMaxHeightOffset;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Build Properties")
	bool bIsDead;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Build Properties")
	TArray<TObjectPtr<UStaticMeshComponent>> BuildExtensionsMeshes;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<USceneComponent> TargetWidgetLocation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Build Properties")
	float DestroyDelay;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Build Properties")
	TObjectPtr<UStaticMesh> BuildExtensionMesh;

	UFUNCTION()
	void OnRep_BuildHealth();

	UFUNCTION(BlueprintImplementableEvent)
	void ShowHealthBar();

	UFUNCTION(BlueprintImplementableEvent)
	void HideHealthBar();

	FTimerHandle HealthBarTimerHandle;
	void HandleHealthBar(bool NewVisible);

	#pragma region Interface functions
	virtual TArray<UBoxComponent*> ReturnCollisionBoxes_Implementation() override;
	virtual void SetProperties_Implementation(UStaticMesh* Mesh, UStaticMesh* ExtensionMesh, const float& Health, const float& MaxHeightOffset) override;
	virtual void DealDamage_Implementation(const float& Damage, const AActor* Agressor) override;
	virtual void ReturnBuildHealth_Implementation(float& OutBuildHealth, float& OutMaxBuildHealth) override;

	bool IsTargetable_Implementation(AActor* TargeterActor) const;
	void GetTargetWidgetAttachmentParent_Implementation(USceneComponent*& OutParentComponent, FName& OutSocketName) const;
	FVector GetMeleeAttackSlot_Implementation(const int32& SlotIndex) const;
	FVector GetRangedAttackSlot_Implementation(const int32& SlotIndex) const;
	bool IsCurrentMeleeSquadSlotAvailable_Implementation() const;
	bool IsCurrentRangedSquadSlotAvailable_Implementation() const;
	void FreeCurrentRangedSquadSlot_Implementation();
	void FreeCurrentMeleeSquadSlot_Implementation();
	void SetCurrentRangedSquadSlot_Implementation(AWOGBaseSquad* NewSquad);
	void SetCurrentMeleeSquadSlot_Implementation(AWOGBaseSquad* NewSquad);
	#pragma endregion

	bool Trace(const TObjectPtr<UPrimitiveComponent> Component, float& OutDistance);

	UFUNCTION()
	void HandleDamage(const float& Damage, const AActor* Agressor);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_DestroyBuild(const AActor* Agressor);

	void DestroyBuild();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FVector3DWithWidget> MeleeSlots;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FVector3DWithWidget> RangedSlots;

	UPROPERTY(Replicated, VisibleAnywhere)
	TObjectPtr<AWOGBaseSquad> CurrentMeleeSquad;

	UPROPERTY(Replicated, VisibleAnywhere)
	TObjectPtr<AWOGBaseSquad> CurrentRangedSquad;

	UPROPERTY(Replicated)
	TObjectPtr<UWOGSpawnCosmetics> CosmeticsDataAsset;

	void HandleSpawnCosmetics();
	void HandleDestroyCosmetics();
	void ExecuteGameplayCueWithCosmeticsDataAsset(const FGameplayTag& CueTag);

public:

	UFUNCTION(BlueprintCallable, Server, reliable)
	void Server_BuildExtensions();

	UFUNCTION(BlueprintImplementableEvent)
	void HandleChaosDestruction();

	UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable)
	void SetCurrentRangedSquad(AWOGBaseSquad* NewSquad);

	UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable)
	void SetCurrentMeleeSquad(AWOGBaseSquad* NewSquad);

	UFUNCTION(BlueprintPure)
	FORCEINLINE AWOGBaseSquad* GetCurrentRangedSquad() const { return CurrentRangedSquad; }
	UFUNCTION(BlueprintPure)
	FORCEINLINE AWOGBaseSquad* GetCurrentMeleeSquad() const { return CurrentMeleeSquad; }

	UFUNCTION(BlueprintPure)
	FORCEINLINE UWOGSpawnCosmetics* GetCosmeticsDataAsset() const { return CosmeticsDataAsset; }
	FORCEINLINE void SetCosmeticsDataAsset(UWOGSpawnCosmetics* NewAsset) { CosmeticsDataAsset = NewAsset; }
};
