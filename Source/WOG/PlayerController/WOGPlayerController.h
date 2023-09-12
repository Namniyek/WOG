// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "WOGPlayerController.generated.h"

/**
 * 
 */
class UWOGAbilityWidget;

UCLASS()
class WOG_API AWOGPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	int32 UserIndex;

	UFUNCTION(Client, Reliable)
	void Client_CreateAnnouncementWidget(ETimeOfDay NewTOD);

	UFUNCTION()
	void CreateWarningWidget(const FString& Attribute);

	UFUNCTION(Client, Reliable)
	void Client_CreateEndgameWidget();

	UFUNCTION(Client, Reliable)
	void Client_CreateAbilityWidget(const int32& AbilityID, TSubclassOf<UUserWidget> Class, UTexture2D* Icon, const float& Cooldown, const FGameplayTag& Tag);

	UFUNCTION(Client, Reliable)
	void Client_RemoveAbilityWidget(const int& AbilityID);

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

	TObjectPtr<UWOGAbilityWidget> AbilityWidgetOne = nullptr;
	TObjectPtr<UWOGAbilityWidget> AbilityWidgetTwo = nullptr;
	TObjectPtr<UWOGAbilityWidget> AbilityWidgetThree = nullptr;
	TObjectPtr<UWOGAbilityWidget> AbilityWidgetFour = nullptr;

public:
	UFUNCTION(BlueprintPure)
	FORCEINLINE bool GetIsAttacker() { return bIsAttacker; }
	FORCEINLINE void SetDefaultPawn(ABasePlayerCharacter* PawnToSet) { DefaultPawn = PawnToSet; }
	FORCEINLINE TObjectPtr<ABasePlayerCharacter> GetDefaultPawn() { return DefaultPawn; }

	
};
