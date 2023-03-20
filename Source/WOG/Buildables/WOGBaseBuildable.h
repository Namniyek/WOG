// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "WOG/Interfaces/BuildingInterface.h"
#include "WOGBaseBuildable.generated.h"

/**
 * 
 */
UCLASS()
class WOG_API AWOGBaseBuildable : public AStaticMeshActor, public IBuildingInterface
{
	GENERATED_BODY()

public:
	AWOGBaseBuildable();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Build Properties")
	float BuildHealth;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Build Properties")
	float BuildMaxHeightOffset;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Build Properties")
	bool bIsDead;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Build Properties")
	TArray <TObjectPtr<AActor>> BuildChildren;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Build Properties")
	TArray<TObjectPtr<UStaticMeshComponent>> BuildExtensionsMeshes;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Build Properties")
	float DestroyDelay;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Build Properties")
	TObjectPtr<UStaticMesh> BuildExtensionMesh;


	//Interface functions
	
	virtual TArray<UBoxComponent*> ReturnCollisionBoxes_Implementation() override;

	virtual void SetProperties_Implementation(UStaticMesh* Mesh, UStaticMesh* ExtensionMesh, const float& Health, const float& MaxHeightOffset) override;

	virtual void DealDamage_Implementation(const float& Damage) override;

	virtual void AddBuildChild_Implementation(AActor* Actor) override;

	//

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_DestroyBuild();

	void DestroyBuild();

	bool Trace(const TObjectPtr<UPrimitiveComponent> Component, float& OutDistance);



public:

	UFUNCTION(BlueprintCallable)
	void BuildExtensions();

	UFUNCTION(BlueprintImplementableEvent)
	void HandleChaosDestruction();
};
