// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WOG/Types/CharacterTypes.h"
#include "WOGBaseAbility.generated.h"


class UAnimMontage;
class USoundCue;
class ABasePlayerCharacter;
class UNiagaraSystem;




UCLASS()
class WOG_API AWOGBaseAbility : public AActor
{
	GENERATED_BODY()
	
public:	
	AWOGBaseAbility();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;
	virtual void Equip();
	virtual void Unequip();
	virtual void Use();
	virtual void CosmeticUse();
	void ResetCooldown();

	#pragma region Base Variables
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Base Variables")
	TObjectPtr<UAnimMontage> EquipMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Base Variables")
	TObjectPtr<UAnimMontage> UseMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Base Variables")
	TObjectPtr<USoundCue> IdleSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Base Variables")
	TObjectPtr<USoundCue> UseSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Base Variables")
	TObjectPtr<UNiagaraSystem> IdleParticleSystem;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Base Variables")
	TObjectPtr<UNiagaraSystem> UseParticleSystem;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Base Variables")
	float CooldownTime;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Base Variables")
	EAbilityType AbilityType;
	#pragma endregion

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Base Variables")
	TObjectPtr<ABasePlayerCharacter> OwnerCharacter;

	UPROPERTY(Replicated)
	bool bIsInCooldown;

	FTimerHandle CooldownTimer;

public:	
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(Server, reliable)
	void Server_Equip();

	UFUNCTION(Server, reliable)
	void Server_Unequip();

	UFUNCTION(Server, reliable)
	void Server_Use();

	UFUNCTION(NetMulticast, reliable)
	void Multicast_CosmeticUse();

	FORCEINLINE void SetOwnerCharacter(ABasePlayerCharacter* NewOwner) { OwnerCharacter = NewOwner; };
	FORCEINLINE EAbilityType GetAbilityType() const { return AbilityType; }
	FORCEINLINE bool GetIsInCooldown() const { return bIsInCooldown; }

};
