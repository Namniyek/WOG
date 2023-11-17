// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTags.h"
#include "Types/CharacterTypes.h"
#include "Interfaces/VendorInterface.h"
#include "WOGStashBase.generated.h"

class UWOGVendorItem;
class UCameraComponent;
class USphereComponent;
class UWidgetComponent;
class UAGR_InventoryManager;
class ABasePlayerCharacter;

UCLASS()
class WOG_API AWOGStashBase : public AActor
{
	GENERATED_BODY()
	
public:	
	AWOGStashBase();

	void SetIsBusy(const bool& NewBusy, ABasePlayerCharacter* UserPlayer);

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

public:	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Setup)
	FName CommonInventoryTag;

};
