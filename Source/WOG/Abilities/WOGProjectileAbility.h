// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WOGBaseAbility.h"
#include "WOGProjectileAbility.generated.h"

/**
 * 
 */
class UUserWidget;
class UNiagaraComponent;

UCLASS()
class WOG_API AWOGProjectileAbility : public AWOGBaseAbility
{
	GENERATED_BODY()
public:
	AWOGProjectileAbility();


protected:
	#pragma region Functions
	virtual void BeginPlay() override;
	virtual void Equip() override;
	virtual void Unequip() override;
	virtual void Use();
	virtual void CosmeticUse();

	void HandleCosmeticEquip();
	void HandleCosmeticUnequip();

	//Line trace for item under the crosshairs
	bool TraceUnderCrosshairs(FHitResult& OutHitResult, FVector& OutHitLocation);
	void GetBeamEndLocation(const FVector& StartLocation, FHitResult& OutHitResult);
	#pragma endregion

	#pragma region RPCs
	UFUNCTION(Client, reliable)
	void Client_AddCrosshair();
	UFUNCTION(Client, reliable)
	void Client_RemoveCrosshair();

	UFUNCTION(NetMulticast, reliable)
	void Multicast_HandleCosmeticEquip();
	UFUNCTION(NetMulticast, reliable)
	void Multicast_HandleCosmeticUnequip();
	#pragma endregion

	#pragma region Base Variables
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Base Variables")
	TSubclassOf<UUserWidget> CrosshairWidgetClass;

	TObjectPtr<UUserWidget> CrosshairWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Base Variables")
	TSubclassOf<AActor> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Base Variables")
	TObjectPtr<UNiagaraSystem> ImpactParticleSystem;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX")
	TObjectPtr<UNiagaraComponent> IdleSystemLeft;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX")
	TObjectPtr<UNiagaraComponent> IdleSystemRight;
	#pragma endregion

public:

	
};
