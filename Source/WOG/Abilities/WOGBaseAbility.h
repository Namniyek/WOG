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
class UWOGAbilityDataAsset;


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

	virtual void Init();

	virtual void CosmeticUse();
	void ResetCooldown();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup | Data Asset")
	TObjectPtr<UWOGAbilityDataAsset> AbilityDataAsset;

	#pragma region Base Variables
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Setup | Base")
	TObjectPtr<UAnimMontage> EquipMontage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Setup | Base")
	TObjectPtr<UAnimMontage> UseMontage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Setup | Base")
	TObjectPtr<USoundCue> IdleSound;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Setup | Base")
	TObjectPtr<USoundCue> UseSound;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Setup | Base")
	TObjectPtr<UNiagaraSystem> IdleParticleSystem;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Setup | Base")
	TObjectPtr<UNiagaraSystem> UseParticleSystem;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Setup | Base")
	float CooldownTime;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Setup | Base")
	EAbilityType AbilityType;
	#pragma endregion

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<ABasePlayerCharacter> OwnerCharacter;

	UPROPERTY(Replicated)
	bool bIsInCooldown;

	FTimerHandle CooldownTimer;

public:	
	virtual void Tick(float DeltaTime) override;

	virtual void CosmeticEquip();
	virtual void CosmeticUnequip();

	UFUNCTION(Server, reliable)
	void Server_Equip();
	UFUNCTION(Server, reliable)
	void Server_Unequip();
	UFUNCTION(Server, reliable)
	virtual void Server_Use();

	UFUNCTION(NetMulticast, reliable)
	void Multicast_CosmeticUse();
	UFUNCTION(NetMulticast, reliable)
	void Multicast_CosmeticEquip();
	UFUNCTION(NetMulticast, reliable)
	void Multicast_CosmeticUnequip();

	FORCEINLINE void SetOwnerCharacter(ABasePlayerCharacter* NewOwner) { OwnerCharacter = NewOwner; };
	FORCEINLINE EAbilityType GetAbilityType() const { return AbilityType; }
	FORCEINLINE bool GetIsInCooldown() const { return bIsInCooldown; }

};
