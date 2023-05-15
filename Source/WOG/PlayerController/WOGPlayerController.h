// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "WOGPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class WOG_API AWOGPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	int32 UserIndex;

	UFUNCTION(Client, Reliable)
	void Client_CreateAnnouncementWidget(ETimeOfDay NewTOD);

	UFUNCTION(Client, Reliable)
	void Client_CreateEndgameWidget();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(Server, reliable)
	void PossessMinion(AActor* ActorToPossess);
	UFUNCTION(Server, reliable)
	void UnpossessMinion();

	virtual void AcknowledgePossession(class APawn* P);

	UFUNCTION()
	void Test(APawn* NewPawn);

protected:
	virtual void OnPossess(APawn* aPawn) override;
	virtual void BeginPlay() override;

private:
	UFUNCTION(Server, reliable)
	void Server_SetPlayerIndex(int32 NewIndex);

	class AWOGMatchHUD* MatchHUD;

	UPROPERTY()
	ETimeOfDay TOD;

	void SetTODString(ETimeOfDay CurrentTOD, FString &StringMain, FString &StringSec);

	UPROPERTY(VisibleAnywhere, Replicated)
	bool bIsAttacker = false; 

	UFUNCTION(Client, Reliable)
	void Client_ResetHUD();

	TObjectPtr<class ABasePlayerCharacter> DefaultPawn = nullptr;

public:
	UFUNCTION(BlueprintPure)
	FORCEINLINE bool GetIsAttacker() { return bIsAttacker; }
	FORCEINLINE void SetDefaultPawn(ABasePlayerCharacter* PawnToSet) { DefaultPawn = PawnToSet; }
	FORCEINLINE TObjectPtr<ABasePlayerCharacter> GetDefaultPawn() { return DefaultPawn; }

	
};
