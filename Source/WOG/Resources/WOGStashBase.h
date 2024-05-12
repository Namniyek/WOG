// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTags.h"
#include "GameFramework/Actor.h"
#include "Interfaces/InventoryInterface.h"
#include "WOGStashBase.generated.h"

class UWOGVendorItem;
class UCameraComponent;
class USphereComponent;
class UWidgetComponent;
class UAGR_InventoryManager;

UCLASS()
class WOG_API AWOGStashBase : public AActor, public IInventoryInterface
{
	GENERATED_BODY()
	
public:	
	friend class ABasePlayerCharacter;
	AWOGStashBase();

	void SetIsBusy(const bool& NewBusy, ABasePlayerCharacter* UserPlayer);

	virtual void BackFromWidget_Implementation(AActor* Actor) override;

	virtual void SwitchItem_Implementation(bool bToCommon, AActor* ItemToSwitch, AActor* PreviousItem, FGameplayTagContainer AuxTagsContainer, TSubclassOf<AActor> ItemClass, const int32& Amount) override;

	void SwitchStashedItems(const bool& bToCommon, AActor* ItemToSwitch, AActor* PreviousItem, FGameplayTagContainer AuxTagsContainer, TSubclassOf<AActor> ItemClass, const int32& Amount);
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<USphereComponent> OverlapSphere;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UWidgetComponent> InteractWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UWidgetComponent> BusyWidget;

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

	UPROPERTY(VisibleAnywhere, Replicated)
	TArray<ABasePlayerCharacter*> OverlappingPlayers = {};

	bool bIsPlayerOverlapping;

	UPROPERTY()
	TObjectPtr<APlayerController> CurrentUserPC;

	UPROPERTY(Replicated)
	TObjectPtr<ABasePlayerCharacter> PlayerUsingStash;

	UFUNCTION()
	void OnInteractWithStashComplete(ABasePlayerCharacter* Interactor);

	UFUNCTION()
	void OnCameraBlendInFinished();
	UFUNCTION()
	void OnCameraBlendOutFinished();

	void FreeStash();

	void ShowCorrectWidget(bool bIsVendorBusy, ABasePlayerCharacter* OverlappingActor);

	UFUNCTION()
	void RefreshStashItems();

	#pragma region TOD
	UFUNCTION()
	void TimeOfDayChanged(const ETimeOfDay TOD);

	UFUNCTION()
	void OnKeyTimeHit(int32 CurrentTime);

	UPROPERTY(Replicated)
	bool bIsDay = true;
	#pragma endregion 

public:	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Setup)
	FName CommonInventoryTag;

	UFUNCTION(BlueprintImplementableEvent)
	void HandleCosmetics(bool bNewBusy);

};
