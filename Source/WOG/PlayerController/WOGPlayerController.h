// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "WOGPlayerController.generated.h"

/**
 * 
 */
class ABasePlayerCharacter;
class UWOGUIManagerComponent;
class UWOGSoundtrackData;

UCLASS()
class WOG_API AWOGPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AWOGPlayerController();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	int32 UserIndex;

	UFUNCTION(Server, reliable)
	void Server_PossessMinion(AActor* ActorToPossess);
	UFUNCTION(Server, reliable)
	void Server_UnpossessMinion(APawn* AIPawnLeft);

	virtual void AcknowledgePossession(class APawn* P);

	void OnNetCleanup(UNetConnection* Connection) override;

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_RequestUnregisterFromSession(APlayerController* UserToUnregister);

protected:
	virtual void OnPossess(APawn* aPawn) override;
	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TObjectPtr<UWOGUIManagerComponent> UIManagerComponent;

	#pragma region Soundtrack DataAssets
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Soundtrack")
	TObjectPtr<UWOGSoundtrackData> SoundtrackDataAsset;
	#pragma endregion

private:
	UFUNCTION(Server, reliable)
	void Server_SetPlayerIndex(int32 NewIndex);

	UPROPERTY(VisibleAnywhere, Replicated)
	bool bIsAttacker = false; 

	TObjectPtr<ABasePlayerCharacter> DefaultPawn = nullptr;

	UFUNCTION()
	void FinishUnPossess(APawn* PawnToPossess, APawn* AIPawnLeft);

public:
	UFUNCTION(BlueprintPure)
	FORCEINLINE bool GetIsAttacker() const { return bIsAttacker; }
	FORCEINLINE void SetDefaultPawn(ABasePlayerCharacter* PawnToSet) { DefaultPawn = PawnToSet; }
	FORCEINLINE TObjectPtr<ABasePlayerCharacter> GetDefaultPawn() { return DefaultPawn; }

	UFUNCTION(BlueprintPure)
	FORCEINLINE UWOGUIManagerComponent* GetUIManagerComponent() const { return UIManagerComponent; }
};
