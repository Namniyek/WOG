// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/WOGBaseInventoryItem.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "Data/WOGDataTypes.h"
#include "Components/StaticMeshComponent.h"
#include "GameplayTagContainer.h"
#include "GameplayAbilitySpec.h"
#include "WOGBaseWeapon.generated.h"


class UAnimMontage;
class USoundCue;
class UGameplayEffect;
class UAGR_ItemComponent;
class USphereComponent;
class AWOGRangedWeaponBase;
class AActor;
class UCameraShakeBase;


USTRUCT(BlueprintType)
struct FWeaponDataTable : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1 - Base")
	bool bIsAttacker = false;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3 - GAS")
	FGameplayTag CooldownTag = FGameplayTag();

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "4 - Stats")
	float Cooldown = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "4 - Stats")
	TEnumAsByte<ETraceTypeQuery> TraceChannel = ETraceTypeQuery::TraceTypeQuery6;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "5 - Cosmetic")
	USoundCue* SwingSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "5 - Cosmetic")
	USoundCue* HitSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "5 - Cosmetic")
	USoundCue* AOEImpactSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "5 - Cosmetic")
	USoundCue* BlockSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "5 - Cosmetic")
	TSubclassOf<UCameraShakeBase> AOECameraShake = nullptr;
	//TO-DO SFX particles for weapon trail && hit FX

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "6 - User Interface")
	TSubclassOf<UUserWidget> AbilityWidgetClass = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "6 - User Interface")
	UTexture2D* AbilityIcon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "6 - User Interface")
	FVendorItemData VendorItemData = FVendorItemData();
};

UCLASS()
class WOG_API AWOGBaseWeapon : public AWOGBaseInventoryItem
{
	GENERATED_BODY()
	friend class ABasePlayerCharacter;

public:	

	AWOGBaseWeapon();
	virtual void PostInitializeComponents();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;
	virtual void InitData() override;
	void UpdateVendorData(FWeaponDataTable* Row);

	#pragma region ActorComponents
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	USphereComponent* SphereComponent;
	#pragma endregion

	#pragma region WeaponVariables

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UStaticMeshComponent* MeshMain;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UStaticMeshComponent* MeshSecondary;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FWeaponDataTable WeaponData;

	UPROPERTY()
	FTimerHandle CatchRangedWeaponTimerHandle;
	#pragma endregion

	UFUNCTION()
	void OnWeaponOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	void OnItemEquipped(AActor* User, FName SlotName) override;

	UFUNCTION(NetMulticast, reliable)
	void Multicast_OnWeaponEquip(AActor* User, FName SlotName);

	virtual bool GrantAbilities(AActor* User) override;
	virtual bool RemoveGrantedAbilities(AActor* User) override;

	virtual void StoreWeapon(const FName& Key);
	virtual void RestoreWeapon(ABasePlayerCharacter* NewOwner);

	#pragma region Drop weapon functionality

	virtual void InitWeaponDefaults();
	FTransform MeshMainOriginalTransform;
	FTransform MeshSecOriginalTransform;

	UFUNCTION(BlueprintCallable)
	virtual void DropWeapon();

	#pragma endregion

private:
	float TimeSinceBlockStarted;
	void SetTraceMeshes(const FName& Slot, AActor* OwnerActor);

	UPROPERTY(VisibleAnywhere)
	int32 ComboStreak;

	TArray<AActor*> HitActorsToIgnore;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<AWOGRangedWeaponBase> SpawnedRangedWeapon;

	void AddAbilityWidget(const int32& Key);
	UPROPERTY(Replicated)
	int32 AbilityKey = 1;

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

	UFUNCTION(BlueprintPure)
	FORCEINLINE FWeaponDataTable GetWeaponData() const { return WeaponData; }

	UFUNCTION(BlueprintPure)
	FORCEINLINE int32 GetComboStreak() const { return ComboStreak; }

	void StartCatchRangedWeaponTimer();
};
