// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "WOG/Types/CharacterTypes.h"
#include "WOGAbilityDataAsset.generated.h"

/**
 * 
 */
class UAnimMontage;
class USoundCue;
class UNiagaraSystem;

UCLASS()
class WOG_API UWOGAbilityDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:

	#pragma region Base Variables
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup | Base")
	EAbilityType AbilityType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup | Base")
	float CooldownTime;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup | Base")
	float Value;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup | Base")
	float Duration;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup | Base")
	TObjectPtr<UAnimMontage> EquipMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup | Base")
	TObjectPtr<UAnimMontage> UseMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup | Base")
	TObjectPtr<USoundCue> IdleSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup | Base")
	TObjectPtr<USoundCue> UseSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup | Base")
	TObjectPtr<UNiagaraSystem> IdleParticleSystem;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup | Base")
	TObjectPtr<UNiagaraSystem> UseParticleSystem;

	#pragma endregion

	#pragma region Projectile
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup | Projectile")
	TSubclassOf<AActor> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup | Projectile")
	TSubclassOf<UUserWidget> CrosshairWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup | Projectile")
	TObjectPtr<UNiagaraSystem> ImpactParticleSystem;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup | Projectile")
	TObjectPtr<USoundCue> ImpactSound;
	#pragma endregion

	#pragma region AOE

	#pragma endregion

};
