// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTags.h"
#include "WOGBaseActivitySlot.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnActivitySlotEntered, AWOGBaseActivitySlot*, Slot, AActor*, NewActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnActivitySlotExited, bool, bSuccess, AWOGBaseActivitySlot*, Slot, AActor*, PreviousActor);

class UWOGAnimationCollectionData;
class USoundCue;
class UNiagaraSystem;

UCLASS()
class WOG_API AWOGBaseActivitySlot : public AActor
{
	GENERATED_BODY()
	
public:	
	AWOGBaseActivitySlot();
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup")
	FGameplayTag ActivitySlotTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup")
	FVector2D MinMaxDuration;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Setup")
	TObjectPtr<UAnimMontage> ActionMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup")
	TObjectPtr<UWOGAnimationCollectionData> AnimationCollection;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup")
	bool bIsStatic;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup")
	FName ActionPropSocket;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup")
	TSubclassOf<AActor> ActionPropClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup|Cosmetics")
	TObjectPtr<USoundCue> ActionSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup|Cosmetics")
	TObjectPtr<UNiagaraSystem> ActionParticleFX;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup|Cosmetics")
	FName ActionParticleFXSocket;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnActivitySlotEntered OnActivitySlotEnteredDelegate;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnActivitySlotExited OnActivitySlotExitedDelegate;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
	bool bIsSlotReserved;

	#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	#endif

private:

	void UpdateActionMontage();

public:	
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	FORCEINLINE void SetIsSlotReserved(const bool& bNewReserved) { bIsSlotReserved = bNewReserved; }
};
