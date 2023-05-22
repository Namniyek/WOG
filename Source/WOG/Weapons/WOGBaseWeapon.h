// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "WOG/Types/CharacterTypes.h"
#include "Components/StaticMeshComponent.h"
#include "GameplayTagContainer.h"
#include "WOGBaseWeapon.generated.h"


class UAnimMontage;
class USoundCue;
class ABasePlayerCharacter;
class UDidItHitActorComponent;
class UGameplayEffect;
class UAGR_ItemComponent;
class USphereComponent;


USTRUCT(BlueprintType)
struct FWeaponDataTable : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Data")
	FName WeaponName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Data")
	bool bIsAttacker;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Data")
	EWeaponType WeaponType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Data")
	UStaticMesh* MeshMain;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Data")
	UStaticMesh* MeshSecondary;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Data")
	FName MeshMainSocket;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Data")
	FName MeshSecondarySocket;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Data")
	FName BackMainSocket;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Data")
	FName BackSecondarySocket;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Data")
	FGameplayTag WeaponTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Data")
	FGameplayTag WeaponPoseTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
	UAnimMontage* AttackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
	UAnimMontage* DodgeMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
	UAnimMontage* BlockMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
	UAnimMontage* EquipMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
	UAnimMontage* HurtMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float BaseDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float HeavyDamageMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float DamageMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float ComboDamageMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 MaxComboStreak;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxParryThreshold;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	USoundCue* SwingSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	USoundCue* HitSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	USoundCue* BlockSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Data")
	TSubclassOf<UGameplayEffect> WeaponDamageEffect;

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
	EWS_Blocking UMETA(DisplayName = "Blocking"),

	EWS_MAX UMETA(DisplayName = "DefaultMAX")
};

UCLASS()
class WOG_API AWOGBaseWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	AWOGBaseWeapon();
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void PostInitializeComponents();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;

	#pragma region ActorComponents

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UDidItHitActorComponent* TraceComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UAGR_ItemComponent* ItemComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	USphereComponent* SphereComponent;
	#pragma endregion

	#pragma region WeaponVariables

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UStaticMeshComponent* MeshMain;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UStaticMeshComponent* MeshSecondary;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName WeaponName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FWeaponDataTable WeaponData;

	#pragma endregion

	UFUNCTION()
	void OnWeaponOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnWeaponPickedUp(UAGR_InventoryManager* Inventory);

	UFUNCTION()
	void OnWeaponEquip(AActor* User, FName SlotName);
	UFUNCTION(NetMulticast, reliable)
	void Multicast_OnWeaponEquip(AActor* User, FName SlotName);

	UFUNCTION()
	void OnWeaponUnequip(AActor* User, FName SlotName);


private:
	void InitWeaponData();

	float TimeSinceBlockStarted;
	void Equip();
	void Unequip();

	UFUNCTION(BlueprintCallable)
	void AttackLight();
	UFUNCTION(BlueprintCallable)
	void AttackHeavy();
	UFUNCTION(BlueprintCallable)
	void Block();
	UFUNCTION(BlueprintCallable)
	void StopBlocking();

	UPROPERTY(VisibleAnywhere)
	int32 ComboStreak;
	UPROPERTY(VisibleAnywhere)
	bool bAttackWindowOpen;
	UPROPERTY(Replicated, VisibleAnywhere)
	bool bIsBlocking;
	UPROPERTY(Replicated, VisibleAnywhere)
	bool bCanParry;
	FTimerHandle ParryTimer;

	void SetCanNotParry();

	UPROPERTY(Replicated, VisibleAnywhere)
	bool bIsArmingHeavy;

	UPROPERTY(ReplicatedUsing = OnRep_WeaponStateChanged, VisibleAnywhere)
	EWeaponState WeaponState;

	UFUNCTION()
	void OnRep_WeaponStateChanged();

	FORCEINLINE void SetWeaponState(EWeaponState NewWeaponState) { WeaponState = NewWeaponState; }

	TArray<AActor*> HitActorsToIgnore;

public:	
	void InitTraceComponent();

	UFUNCTION(BlueprintCallable)
	void AttachToBack();
	UFUNCTION(BlueprintCallable)
	void AttachToHands();

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

	UFUNCTION()
	void HitDetected(FHitResult Hit);

	UFUNCTION(BlueprintCallable)
	void IncreaseCombo();

	UFUNCTION(BlueprintCallable)
	void ResetCombo();

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<ABasePlayerCharacter> OwnerCharacter;

	UFUNCTION(BlueprintPure)
	FORCEINLINE EWeaponState GetWeaponState() const { return WeaponState; }
	UFUNCTION(BlueprintPure)
	FORCEINLINE FWeaponDataTable GetWeaponData() const { return WeaponData; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool GetIsBlocking() const { return bIsBlocking; }
	UFUNCTION(BlueprintPure)
	FORCEINLINE int32 GetComboStreak() const { return ComboStreak; }
	FORCEINLINE bool GetIsInCombo() const { return bAttackWindowOpen; }
	FORCEINLINE bool GetCanParry() const { return bCanParry; }
	FORCEINLINE bool GetIsArmingHeavy() const { return bIsArmingHeavy; }
	FORCEINLINE UDidItHitActorComponent* GetTraceComponent() const { return TraceComponent; }

	UFUNCTION(BlueprintCallable)
	void SetOwnerCharacter(ABasePlayerCharacter* NewOwner);
	UFUNCTION(BlueprintPure)
	FORCEINLINE ABasePlayerCharacter* GetOwnerCharacter() const { return OwnerCharacter; }
};
