// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTags.h"
#include "Types/CharacterTypes.h"
#include "Interfaces/VendorInterface.h"
#include "WOGVendor.generated.h"

class UWOGVendorItem;
class UCameraComponent;
class USphereComponent;
class UWidgetComponent;
class UAGR_InventoryManager;
class ABasePlayerCharacter;

UCLASS()
class WOG_API AWOGVendor : public AActor, public IVendorInterface
{
	GENERATED_BODY()
	
public:	
	AWOGVendor();

	void Sell(const TArray<FCostMap>& CostMap, TSubclassOf<AActor> ItemClass, const int32& Amount);

	void SetIsBusy(const bool& NewBusy, ABasePlayerCharacter* UserPlayer);

	void BackFromVendorWidget_Implementation(AActor* Actor);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<USkeletalMeshComponent> Mesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<USphereComponent> OverlapSphere;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UWidgetComponent> InteractWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UWidgetComponent> BusyWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAGR_InventoryManager> VendorInventory;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated)
	TObjectPtr<UAGR_InventoryManager> CommonInventory;

private:
	UFUNCTION()
	void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UPROPERTY(ReplicatedUsing = OnRep_IsBusy)
	bool bIsBusy;

	UFUNCTION()
	void OnRep_IsBusy();

	void PopulateInventory();

	UPROPERTY(VisibleAnywhere, Replicated)
	TArray<ABasePlayerCharacter*> OverlappingPlayers = {};

	bool bIsPlayerOverlapping;

	TObjectPtr<APlayerController> CurrentUserPC;

	UPROPERTY(Replicated)
	TObjectPtr<ABasePlayerCharacter> PlayerUsingVendor;

	UFUNCTION()
	void OnInteractWithVendorComplete(ABasePlayerCharacter* Interactor);

	UFUNCTION()
	void OnCameraBlendInFinished();
	UFUNCTION()
	void OnCameraBlendOutFinished();

	void FreeVendor();

	void ShowCorrectWidget(bool bIsVendorBusy, ABasePlayerCharacter* OverlappingActor);

	UFUNCTION()
	void RefreshVendorItems();

public:	

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Setup)
	FGameplayTag ItemTypeFilter;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Setup)
	FGameplayTag VendorTypeFilter;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Setup)
	TMap<TSubclassOf<AActor>, int32> DefaultInventoryMap;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Setup)
	TSubclassOf<UWOGVendorItem> VendorItemClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Setup)
	FName CommonInventoryTag;
};
