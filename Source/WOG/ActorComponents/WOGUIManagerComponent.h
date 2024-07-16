// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WOGUIManagerComponent.generated.h"

struct FGameplayTag;
class UWOGUIManagerSubsystem;
class AWOGHuntEnemy;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WOG_API UWOGUIManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UWOGUIManagerComponent();

	friend class AWOGPlayerController;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	TObjectPtr<AWOGPlayerController> OwnerPC;
	TObjectPtr<UWOGUIManagerSubsystem> UIManager;

public:	

	UFUNCTION(Client, Reliable)
	void Client_AddAbilityWidget(const int32& AbilityID, TSubclassOf<UUserWidget> Class, UTexture2D* Icon, const float& Cooldown, const FGameplayTag& Tag);

	UFUNCTION(Client, Reliable)
	void Client_RemoveAbilityWidget(const int32& AbilityID);
		
	UFUNCTION(Client, Reliable)
	void Client_AddAnnouncementWidget(ETimeOfDay NewTOD);

	UFUNCTION(Client, Reliable)
	void Client_AddEndgameWidget();

	UFUNCTION(Client, Reliable)
	void Client_ResetHUD();

	UFUNCTION(Client, Reliable, BlueprintCallable)
	void Client_AddBarsWidget();
	UFUNCTION(Client, Reliable, BlueprintCallable)
	void Client_RemoveBarsWidget();

	UFUNCTION(Client, Reliable, BlueprintCallable)
	void Client_AddMinimapWidget();
	UFUNCTION(Client, Reliable, BlueprintCallable)
	void Client_RemoveMinimapWidget();

	UFUNCTION(Client, Reliable, BlueprintCallable)
	void Client_CollapseAbilitiesWidget();
	UFUNCTION(Client, Reliable, BlueprintCallable)
	void Client_RestoreAbilitiesWidget();

	UFUNCTION(Client, Reliable, BlueprintCallable)
	void Client_CollapseTODWidget();
	UFUNCTION(Client, Reliable, BlueprintCallable)
	void Client_RestoreTODWidget();

	UFUNCTION(Client, Reliable, BlueprintCallable)
	void Client_CollapseObjectiveWidget();
	UFUNCTION(Client, Reliable, BlueprintCallable)
	void Client_RestoreObjectiveWidget();

	UFUNCTION(Client, Reliable, BlueprintCallable)
	void Client_AddAvailableResourceWidget();
	UFUNCTION(Client, Reliable, BlueprintCallable)
	void Client_RemoveAvailableResourceWidget();

	UFUNCTION(Client, Reliable, BlueprintCallable)
	void Client_UpdateVendorWidget();

	UFUNCTION(Client, Reliable, BlueprintCallable)
	void Client_CreateGenericWarningWidget(const FString& WarningString);

	UFUNCTION(Client, Reliable, BlueprintCallable)
	void Client_AddHuntWidget(AWOGHuntEnemy* HuntEnemy);
	UFUNCTION(Client, Reliable, BlueprintCallable)
	void Client_RemoveHuntWidget();

	UFUNCTION(Client, Reliable, BlueprintCallable)
	void Client_AddFloatingDamageText(const float& DamageAmount, AActor* TargetActor);

	UFUNCTION(Client, Reliable, BlueprintCallable)
	void Client_AddCharacterHealthBarWidget(float NewValue, float MaxValue, AActor* TargetActor);
};
