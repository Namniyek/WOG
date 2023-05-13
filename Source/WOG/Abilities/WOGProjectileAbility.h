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
class USoundCue;

UCLASS()
class WOG_API AWOGProjectileAbility : public AWOGBaseAbility
{
	GENERATED_BODY()
public:
	AWOGProjectileAbility();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	#pragma region Functions
	virtual void BeginPlay() override;
	virtual void Equip() override;
	virtual void Unequip() override;
	virtual void Use() override;
	virtual void Init() override;


	//Line trace for item under the crosshairs
	bool TraceUnderCrosshairs(FHitResult& OutHitResult, FVector& OutHitLocation);
	void GetBeamEndLocation(const FVector& TraceStartLocation, FHitResult& OutHitResult);
	#pragma endregion

	#pragma region RPCs
	UFUNCTION(Client, reliable)
	void Client_AddCrosshair();
	UFUNCTION(Client, reliable)
	void Client_RemoveCrosshair();

	#pragma endregion

	#pragma region Base Variables
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup | Projectile")
	TSubclassOf<UUserWidget> CrosshairWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup | Projectile")
	TSubclassOf<AActor> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup | Projectile")
	TObjectPtr<UNiagaraSystem> ImpactParticleSystem;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Setup | Projectile")
	TObjectPtr<USoundCue> ImpactSound;
	#pragma endregion

	TObjectPtr<UUserWidget> CrosshairWidget;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UNiagaraComponent> IdleSystemLeft;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UNiagaraComponent> IdleSystemRight;

	//UPROPERTY(Replicated)
	FVector StartLocation;
	UPROPERTY(Replicated)
	FVector EndLocation;

private:
	FVector CalculateEndVector();

public:
	virtual void CosmeticUse() override;
	virtual void CosmeticEquip() override;
	virtual void CosmeticUnequip() override;
};
