// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/ResourcesInterface.h"
#include "Interfaces/InventoryInterface.h"
#include "WOGResourceBase.generated.h"

class UAGR_ItemComponent;
class ABasePlayerCharacter;
class USphereComponent;
class UWidgetComponent;

UCLASS()
class WOG_API AWOGResourceBase : public AActor, public IResourcesInterface, public IInventoryInterface
{
	GENERATED_BODY()
	
public:	
	AWOGResourceBase();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> Mesh;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAGR_ItemComponent> ItemComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<USphereComponent> OverlapSphere;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UWidgetComponent> InteractWidget;

private:

	UFUNCTION()
	void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void OnInteractWithResourceComplete(ABasePlayerCharacter* Interactor);

	UPROPERTY(VisibleAnywhere, Replicated)
	TArray<ABasePlayerCharacter*> OverlappingPlayers = {};
};
