// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WOGBaseActorComponent.h"
#include "WOGAttributesComponent.generated.h"

/**
 * 
 */

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class WOG_API UWOGAttributesComponent : public UWOGBaseActorComponent
{
	GENERATED_BODY()
public:
	UWOGAttributesComponent();
	friend class ABasePlayerCharacter;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Attributes, meta = (AllowPrivateAccess = "true"), Replicated)
	float MaxHealth;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Attributes, meta = (AllowPrivateAccess = "true"), ReplicatedUsing = "OnRep_Health")
	float Health;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Attributes, meta = (AllowPrivateAccess = "true"), Replicated)
	float HealthPercent;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Attributes, meta = (AllowPrivateAccess = "true"), Replicated)
	float MaxMana;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Attributes, meta = (AllowPrivateAccess = "true"), ReplicatedUsing = "OnRep_Mana")
	float Mana;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Attributes, meta = (AllowPrivateAccess = "true"), Replicated)
	float MaxAdrenaline;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Attributes, meta = (AllowPrivateAccess = "true"), ReplicatedUsing = "OnRep_Adrenaline")
	float Adrenaline;

private:
	UFUNCTION()
	void OnRep_Health();
	
	UFUNCTION()
	void OnRep_Mana();

	UFUNCTION()
	void OnRep_Adrenaline();

	void UpdateHealth(float Value, AController* InstigatedBy);
	void UpdateMana(float Value);
	void UpdateAdranaline(float Value);

	UPROPERTY()
	ABasePlayerCharacter* OwnerCharacter;

	UPROPERTY()
	class AWOGPlayerController* OwnerPC;

public:
	UFUNCTION(BlueprintCallable, Server, reliable)
	void Server_UpdateHealth(float Value, AController* InstigatedBy);
};
