// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "WOGPlayerController.generated.h"

/**
 * 
 */
class UWOGAbilityWidget;
class UWOGHoldProgressBar;
class UWOGRavenMarkerWidget;
class ABasePlayerCharacter;

UCLASS()
class WOG_API AWOGPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	int32 UserIndex;

	UFUNCTION(Client, Reliable)
	void Client_CreateAnnouncementWidget(ETimeOfDay NewTOD);

	UFUNCTION(Client, Reliable)
	void Client_CreateEndgameWidget();

	UFUNCTION(Client, Reliable)
	void Client_CreateAbilityWidget(const int32& AbilityID, TSubclassOf<UUserWidget> Class, UTexture2D* Icon, const float& Cooldown, const FGameplayTag& Tag);

	UFUNCTION(Client, Reliable)
	void Client_RemoveAbilityWidget(const int& AbilityID);

	UFUNCTION(Server, reliable)
	void Server_PossessMinion(AActor* ActorToPossess);
	UFUNCTION(Server, reliable)
	void Server_UnpossessMinion(APawn* AIPawnLeft);

	virtual void AcknowledgePossession(class APawn* P);

	UFUNCTION()
	void AddStaminaWidget();

	UFUNCTION()
	void AddHoldProgressBar();
	UFUNCTION()
	void RemoveHoldProgressBar();

	UFUNCTION()
	void AddRavenMarkerWidget(const int32& Amount);
	UFUNCTION()
	void RemoveRavenMarkerWidget();

	UFUNCTION()
	void AddScreenDamageWidget(const int32& DamageThreshold);

	UFUNCTION(BlueprintCallable)
	void CreateWarningWidget(const FString& Attribute);

	UFUNCTION(BlueprintCallable)
	void CreateGenericWarningWidget(const FString& Attribute);

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

	TObjectPtr<ABasePlayerCharacter> DefaultPawn = nullptr;

	TObjectPtr<UWOGAbilityWidget> AbilityWidgetOne = nullptr;
	TObjectPtr<UWOGAbilityWidget> AbilityWidgetTwo = nullptr;
	TObjectPtr<UWOGAbilityWidget> AbilityWidgetThree = nullptr;
	TObjectPtr<UWOGAbilityWidget> AbilityWidgetFour = nullptr;

	UPROPERTY()
	TObjectPtr<UWOGHoldProgressBar> HoldProgressBarWidget = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UWOGRavenMarkerWidget> RavenMarkerWidget = nullptr;

	UFUNCTION()
	void FinishUnPossess(APawn* PawnToPossess, APawn* AIPawnLeft);

public:
	UFUNCTION(BlueprintPure)
	FORCEINLINE bool GetIsAttacker() const { return bIsAttacker; }
	FORCEINLINE void SetDefaultPawn(ABasePlayerCharacter* PawnToSet) { DefaultPawn = PawnToSet; }
	FORCEINLINE TObjectPtr<ABasePlayerCharacter> GetDefaultPawn() { return DefaultPawn; }
	FORCEINLINE TObjectPtr<UWOGHoldProgressBar> GetHoldProgressBar() const { return HoldProgressBarWidget; }
	FORCEINLINE TObjectPtr<UWOGRavenMarkerWidget> GetRavenMarkerWidget() const { return RavenMarkerWidget; }
};
