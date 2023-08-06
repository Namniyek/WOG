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
class AActor;


USTRUCT(BlueprintType)
struct FWeaponDataTable : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1 - Base")
	UStaticMesh* MeshMain = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1 - Base")
	UStaticMesh* MeshSecondary = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "2 - Animations")
	FName MeshMainSocket = FName("");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "2 - Animations")
	FName MeshSecondarySocket = FName("");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "2 - Animations")
	FName BackMainSocket = FName("");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "2 - Animations")
	FName BackSecondarySocket = FName("");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "2 - Animations")
	UAnimMontage* AttackMontage = nullptr;

	//Used for dual wield weapons. Set as the one handed montage for the same weapon type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "2 - Animations")
	UAnimMontage* AlternativeAttackMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "2 - Animations")
	UAnimMontage* DodgeMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "2 - Animations")
	UAnimMontage* BlockMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "2 - Animations")
	UAnimMontage* EquipMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "2 - Animations")
	UAnimMontage* HurtMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "2 - Animations")
	float AnimationSpeed = 1.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "3 - GAS")
	FGameplayTag WeaponTag = FGameplayTag();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "3 - GAS")
	FGameplayTag WeaponPoseTag = FGameplayTag();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "3 - GAS")
	TArray<TSubclassOf<class UWOGGameplayAbilityBase>> Abilities = { nullptr };

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "3 - GAS")
	FGameplayTagContainer BlockImpactLightTags = FGameplayTagContainer();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "3 - GAS")
	FGameplayTag BlockImpactHeavyTag = FGameplayTag();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "3 - GAS")
	FGameplayTag ParryTag = FGameplayTag();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "3 - GAS")
	FGameplayTag RangedTag = FGameplayTag();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "3 - GAS")
	TSubclassOf<AActor> RangedClass = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "3 - GAS")
	TSubclassOf<UGameplayEffect> WeaponDamageEffect = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "3 - GAS")
	TSubclassOf<UGameplayEffect> RangedWeaponEffect = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "4 - Stats")
	float BaseDamage = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "4 - Stats")
	float HeavyDamageMultiplier = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "4 - Stats")
	float DamageMultiplier = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "4 - Stats")
	float ComboDamageMultiplier = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "4 - Stats")
	int32 MaxComboStreak = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "4 - Stats")
	float StunDuration = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "4 - Stats")
	float Cost = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "5 - Cosmetic")
	USoundCue* SwingSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "5 - Cosmetic")
	USoundCue* HitSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "5 - Cosmetic")
	USoundCue* BlockSound = nullptr;

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

	TArray<AActor*> HitActorsToIgnore;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<AWOGRangedWeaponBase> SpawnedRangedWeapon;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<AActor> SpawnedAOEAttack;

public:	
	UFUNCTION(BlueprintCallable)
	void AttachToBack();
	UFUNCTION(BlueprintCallable)
	void AttachToHands();

	UFUNCTION(BlueprintCallable)
	void IncreaseCombo();

	UFUNCTION(BlueprintCallable)
	void ResetCombo();

	UFUNCTION(Server, reliable, BlueprintCallable)
	void Server_ThrowWeapon(bool IsTargetValid, const FVector_NetQuantize& TargetLocation);

	UFUNCTION(Server, reliable, BlueprintCallable)
	void Server_SpawnWeaponAOE(const FVector_NetQuantize& TargetLocation);

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

	UFUNCTION(BlueprintCallable)
	void SetOwnerCharacter(ABasePlayerCharacter* NewOwner);
	UFUNCTION(BlueprintPure)
	FORCEINLINE ABasePlayerCharacter* GetOwnerCharacter() const { return OwnerCharacter; }
};
