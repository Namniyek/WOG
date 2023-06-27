// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "WOG/Types/CharacterTypes.h"
#include "Components/StaticMeshComponent.h"
#include "GameplayTagContainer.h"
#include "GameplayAbilitySpec.h"
#include "WOGBaseWeapon.generated.h"


class UAnimMontage;
class USoundCue;
class ABasePlayerCharacter;
class UGameplayEffect;
class UAGR_ItemComponent;
class USphereComponent;
class AWOGRangedWeaponBase;


USTRUCT(BlueprintType)
struct FWeaponDataTable : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Data")
	FName WeaponName = FName("");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Data")
	bool bIsAttacker = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Data")
	UStaticMesh* MeshMain = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Data")
	UStaticMesh* MeshSecondary = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Data")
	FName MeshMainSocket = FName("");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Data")
	FName MeshSecondarySocket = FName("");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Data")
	FName BackMainSocket = FName("");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Data")
	FName BackSecondarySocket = FName("");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Data")
	FGameplayTag WeaponTag = FGameplayTag();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Data")
	FGameplayTag WeaponPoseTag = FGameplayTag();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
	UAnimMontage* AttackMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
	UAnimMontage* DodgeMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
	UAnimMontage* BlockMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
	UAnimMontage* EquipMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
	UAnimMontage* HurtMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float BaseDamage = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float HeavyDamageMultiplier = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float DamageMultiplier = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float ComboDamageMultiplier = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 MaxComboStreak = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxParryThreshold = 0.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GAS")
	float StunDuration = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	USoundCue* SwingSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	USoundCue* HitSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	USoundCue* BlockSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Data")
	TSubclassOf<UGameplayEffect> WeaponDamageEffect = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Data")
	TSubclassOf<UGameplayEffect> RangedWeaponEffect = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GAS")
	TArray<TSubclassOf<class UWOGGameplayAbilityBase>> Abilities = { nullptr };

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GAS")
	FGameplayTagContainer BlockImpactLightTags = FGameplayTagContainer();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GAS")
	FGameplayTag BlockImpactHeavyTag = FGameplayTag();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GAS")
	FGameplayTag ParryTag = FGameplayTag();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GAS")
	FGameplayTag RangedTag = FGameplayTag();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GAS")
	float AnimationSpeed = 1.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GAS")
	TSubclassOf<class AWOGRangedWeaponBase> ThrowableClass = nullptr;

	//TO-DO SFX particles for weapon trail && hit FX

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

	bool GrantWeaponAbilities(AActor* User);
	bool RemoveGrantedAbilities(AActor* User);

	TArray<FGameplayAbilitySpecHandle> GrantedAbilities;

private:
	virtual void InitWeaponData();

	float TimeSinceBlockStarted;

	UFUNCTION(BlueprintCallable)
	void AttackLight();
	UFUNCTION(BlueprintCallable)
	void AttackHeavy();
	UFUNCTION(BlueprintCallable)
	void Block();
	UFUNCTION(BlueprintCallable)
	void StopBlocking();

	void SetTraceMeshes(const FName& Slot, AActor* OwnerActor);

	UPROPERTY(VisibleAnywhere)
	int32 ComboStreak;
	UPROPERTY(VisibleAnywhere)
	bool bAttackWindowOpen;
	UPROPERTY(VisibleAnywhere)
	bool bCanParry;
	FTimerHandle ParryTimer;

	void SetCanNotParry();

	TArray<AActor*> HitActorsToIgnore;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<AWOGRangedWeaponBase> SpawnedRangedWeapon;

public:	
	UFUNCTION(BlueprintCallable)
	void AttachToBack();
	UFUNCTION(BlueprintCallable)
	void AttachToHands();

	UFUNCTION(BlueprintCallable)
	void IncreaseCombo();

	UFUNCTION(BlueprintCallable)
	void ResetCombo();

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void ThrowWeapon(bool IsTargetValid, const FVector_NetQuantize& TargetLocation);

	UFUNCTION(Server, reliable, BlueprintCallable)
	void Server_ThrowWeapon(bool IsTargetValid, const FVector_NetQuantize& TargetLocation);

	UFUNCTION(BlueprintCallable)
	void RecallWeapon();

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void CatchWeapon();

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<ABasePlayerCharacter> OwnerCharacter;

	UFUNCTION(BlueprintPure)
	FORCEINLINE FWeaponDataTable GetWeaponData() const { return WeaponData; }

	UFUNCTION(BlueprintPure)
	FORCEINLINE int32 GetComboStreak() const { return ComboStreak; }
	FORCEINLINE bool GetIsInCombo() const { return bAttackWindowOpen; }
	FORCEINLINE bool GetCanParry() const { return bCanParry; }

	UFUNCTION(BlueprintCallable)
	void SetOwnerCharacter(ABasePlayerCharacter* NewOwner);
	UFUNCTION(BlueprintPure)
	FORCEINLINE ABasePlayerCharacter* GetOwnerCharacter() const { return OwnerCharacter; }
};
