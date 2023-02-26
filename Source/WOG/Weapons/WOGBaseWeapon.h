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
	int32 MaxComboStreak;

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

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	EWS_Stored UMETA(DisplayName = "Stored"),
	EWS_BeingEquipped UMETA(DisplayName = "Being Equipped"),
	EWS_BeingStored UMETA(DisplayName = "Being Stored"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),
	EWS_AttackLight UMETA(DisplayName = "AttackLight"),
	EWS_AttackHeavy UMETA(DisplayName = "AttackHeavy"),

	EWS_MAX UMETA(DisplayName = "DefaultMAX")
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
	int32 MaxComboStreak;

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
	void Unequip();
	void AttachToHands();
	void Drop();
	void HandleHit();
	void AttackLight();
	void AttackHeavy();

	UPROPERTY(Replicated, VisibleAnywhere)
	int32 ComboStreak;
	UPROPERTY(Replicated, VisibleAnywhere)
	bool bIsInCombo;

	UPROPERTY(ReplicatedUsing = OnRep_WeaponStateChanged, VisibleAnywhere)
	EWeaponState WeaponState;

	UFUNCTION()
	void OnRep_WeaponStateChanged();

	FORCEINLINE void SetWeaponState(EWeaponState NewWeaponState) { WeaponState = NewWeaponState; }

public:	
	void AttachToBack();

	UFUNCTION(BlueprintCallable)
	void FinishAttacking();

	UFUNCTION(Server, reliable, BlueprintCallable)
	void Server_Equip();

	UFUNCTION(Server, reliable, BlueprintCallable)
	void Server_Unequip();

	UFUNCTION(Server, reliable)
	void Server_Swap();

	UFUNCTION(Server, reliable, BlueprintCallable)
	void Server_SetWeaponState(EWeaponState NewWeaponState);

	UFUNCTION(Server, reliable)
	void Server_AttackLight();

	UFUNCTION(NetMulticast, reliable)
	void Multicast_AttackLight();

	UFUNCTION(Server, reliable)
	void Server_AttackHeavy();

	UFUNCTION(BlueprintCallable)
	void IncreaseCombo();

	UFUNCTION(BlueprintCallable)
	void ResetCombo();

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
	ABasePlayerCharacter* OwnerCharacter;

	UFUNCTION(BlueprintCallable)
	FORCEINLINE EWeaponState GetWeaponState() const { return WeaponState; }
	FORCEINLINE int32 GetComboStreak() const { return ComboStreak; }
	FORCEINLINE bool GetIsInCombo() const { return bIsInCombo; }
};
