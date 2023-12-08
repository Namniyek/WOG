// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/WOGBaseCharacter.h"
#include "WOGBaseDayNPC.generated.h"

/**
 * 
 */
class AWOGBaseActivitySlot;

UCLASS()
class WOG_API AWOGBaseDayNPC : public AWOGBaseCharacter
{
	GENERATED_BODY()

public:
	AWOGBaseDayNPC();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagContainer ActivitySlotsTags;

protected:
	virtual void BeginPlay() override;
	void InitCurrentState();

	virtual void HandleTODChange() override;

	UFUNCTION(BlueprintImplementableEvent)
	void HandleStateChange();

	UFUNCTION()
	void KeyTimeHit(int32 CurrentTime);

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
	EDayNPCState CurrentState;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<AWOGBaseActivitySlot> PreviousSlot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector SpawnLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	int32 CurrentSlotIndex;

public:
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void SetCurrentState(EDayNPCState NewState);

	UFUNCTION(BlueprintPure)
	FORCEINLINE EDayNPCState GetCurrentState() const { return CurrentState; }

	UFUNCTION(BlueprintPure)
	FGameplayTag GetNextActivitySlotTag();

	UFUNCTION(BlueprintCallable)
	void IncrementCurrentSlotIndex();
};
