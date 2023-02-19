// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "WOG/Types/CharacterTypes.h"
#include "Components/StaticMeshComponent.h"
#include "WOGBaseWeapon.generated.h"

class UAnimMontage;
class USoundCue;
class ABasePlayerCharacter;


USTRUCT(BlueprintType)
struct FWeaponDataTable : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName WeaponName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsAttacker;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EWeaponType WeaponType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* MeshMain;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* MeshSecondary;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName MeshMainSocket;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName MeshSecondarySocket;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName BackMainSocket;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName BackSecondarySocket;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* AttackLightMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* AttackHeavyMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* BlockMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* ParryMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* EquipMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* UnequipMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BaseDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HeavyDamageMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DamageMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ComboDamageMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int MaxComboStreak;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxParryThreshold;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundCue* SwingSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundCue* HitSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundCue* BlockSound;

	//TO-DO SFX particles for weapon trail && hit FX

};

UCLASS()
class WOG_API AWOGBaseWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	AWOGBaseWeapon();
	virtual void Tick(float DeltaSeconds);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UStaticMeshComponent* MeshMain;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UStaticMeshComponent* MeshSecondary;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName MeshMainSocket;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName MeshSecondarySocket;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName BackMainSocket;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName BackSecondarySocket;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName WeaponName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsAttacker;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EWeaponType WeaponType;

	UPROPERTY()
	UAnimMontage* AttackLightMontage;

	UPROPERTY()
	UAnimMontage* AttackHeavyMontage;

	UPROPERTY()
	UAnimMontage* BlockMontage;

	UPROPERTY()
	UAnimMontage* ParryMontage;

	UPROPERTY()
	UAnimMontage* EquipMontage;

	UPROPERTY()
	UAnimMontage* UnequipMontage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float BaseDamage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float HeavyDamageMultiplier;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float DamageMultiplier;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float ComboDamageMultiplier;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int MaxComboStreak;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float MaxParryThreshold;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USoundCue* SwingSound;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USoundCue* HitSound;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USoundCue* BlockSound;

private:
	void InitWeapon();

	float TimeSinceBlockStarted;

	void Equip();

public:	

	UFUNCTION()
	void AttachToBack();

	UFUNCTION(Server, reliable, BlueprintCallable)
	void Server_Equip();

	UFUNCTION(NetMulticast, reliable)
	void Multicast_Equip();

	UFUNCTION()
	void Unequip();

	UFUNCTION()
	void Drop();

	UFUNCTION()
	void HandleHit();

	UPROPERTY(Replicated)
	ABasePlayerCharacter* OwnerCharacter;

};
