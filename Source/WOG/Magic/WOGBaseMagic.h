// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/WOGBaseInventoryItem.h"
#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
#include "Data/WOGDataTypes.h"
#include "WOGBaseMagic.generated.h"

class UAnimMontage;
class USoundCue;
class UGameplayEffect;
class USphereComponent;
class UNiagaraSystem;
class UNiagaraComponent;
class UWOGGameplayAbilityBase;
class AWOGBaseIdleMagic;
class AWOGBaseMagicProjectile;
class AWOGBaseMagicAOE;
class AGameplayAbilityTargetActor_GroundTrace;
class UTexture2D;

USTRUCT(BlueprintType)
struct FMagicDataTable : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1 - Base")
	EAbilityType AbilityType = EAbilityType::EAT_Projectile;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1 - Base")
	EAbilityInputType AbilityInputType = EAbilityInputType::EAI_Instant;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1 - Base")
	bool bIsAttacker = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "2 - Animations")
	FName LeftHandSocket = FName("");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "2 - Animations")
	FName RighHandSocket = FName("");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "2 - Animations")
	UAnimMontage* AttackMontage = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "2 - Animations")
	float AnimationSpeed = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "2 - Animations")
	FName CastMontageSection = FName("");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3 - GAS")
	FGameplayTag MagicTag = FGameplayTag();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3 - GAS")
	FGameplayTag MagicPoseTag = FGameplayTag();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3 - GAS")
	TSubclassOf<UGameplayEffect> DamageEffect = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3 - GAS")
	TSubclassOf<UGameplayEffect> SecondaryEffect = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "3 - GAS")
	float SecondaryEffectDuration = 0.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "3 - GAS")
	TArray<TSubclassOf<UWOGGameplayAbilityBase>> AbilitiesToGrant = { nullptr };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3 - GAS")
	FGameplayTag CooldownTag = FGameplayTag();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "4 - Stats")
	float Value = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "4 - Stats")
	float ValueMultiplier = 0.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "4 - Stats")
	float Cost = 0.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "4 - Stats")
	float Cooldown = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "5 - Cosmetic")
	USoundCue* CastSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "5 - Cosmetic")
	USoundCue* ImpactSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "5 - Cosmetic")
	USoundCue* IdleSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "5 - Cosmetic")
	TSubclassOf<UCameraShakeBase> CameraShake = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "5 - Cosmetic", meta = (EditCondition = "AbilityType == EAbilityType::EAT_Projectile", EditConditionHides))
	USoundCue* ProjectileSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "6 - User Interface")
	TSubclassOf<UUserWidget> AbilityWidgetClass = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "6 - User Interface")
	UTexture2D* AbilityIcon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "6 - User Interface")
	FVendorItemData VendorItemData = FVendorItemData();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Projectile", meta = (EditCondition = "AbilityType == EAbilityType::EAT_Projectile", EditConditionHides))
	TSubclassOf<AWOGBaseIdleMagic> IdleProjectileClass = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Projectile", meta = (EditCondition = "AbilityType == EAbilityType::EAT_Projectile", EditConditionHides))
	TSubclassOf<AWOGBaseMagicProjectile> ProjectileClass = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Projectile", meta = (EditCondition = "AbilityType == EAbilityType::EAT_Projectile", EditConditionHides))
	TObjectPtr<UNiagaraSystem> HitVFX = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AOE", meta = (EditCondition = "AbilityType == EAbilityType::EAT_AOE", EditConditionHides))
	TSubclassOf<AWOGBaseIdleMagic> IdleAOEClass = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AOE", meta = (EditCondition = "AbilityType == EAbilityType::EAT_AOE", EditConditionHides))
	TSubclassOf<AWOGBaseMagicAOE> AOEClass = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AOE", meta = (EditCondition = "AbilityType == EAbilityType::EAT_AOE", EditConditionHides))
	TSubclassOf<AGameplayAbilityTargetActor_GroundTrace> AOEMarkerClass = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Instant", meta = (EditCondition = "AbilityType == EAbilityType::EAT_Instant", EditConditionHides))
	float Magnitude = 0.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Instant", meta = (EditCondition = "AbilityType == EAbilityType::EAT_Instant", EditConditionHides))
	float Range = 0.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Buff", meta = (EditCondition = "AbilityType == EAbilityType::EAT_Buff", EditConditionHides))
	float Duration = 0.f;

	//TO-DO SFX particles for weapon trail && hit FX

};

UCLASS()
class WOG_API AWOGBaseMagic : public AWOGBaseInventoryItem
{
	GENERATED_BODY()
	friend class ABasePlayerCharacter;
	
public:	
	AWOGBaseMagic();
	virtual void PostInitializeComponents();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void InitData() override;
	void UpdateVendorData(FMagicDataTable* Row);

	#pragma region ActorComponents

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	USphereComponent* SphereComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UNiagaraComponent* StandbyEffect;
	#pragma endregion

	#pragma region MagicVariables
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FMagicDataTable MagicData;
	#pragma endregion

	#pragma region Item Functions
	UFUNCTION()
	void OnMagicOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	virtual void OnItemEquipped (AActor* User, FName SlotName) override;

	UFUNCTION(NetMulticast, reliable)
	void Multicast_OnMagicEquip(AActor* User, FName SlotName);
	#pragma endregion

	#pragma region GAS
	bool GrantAbilities(AActor* User) override;
	bool RemoveGrantedAbilities(AActor* User) override;
	#pragma endregion

	virtual void StoreMagic(const FName& Key);
	virtual void RestoreMagic(ABasePlayerCharacter* NewOwner);

	#pragma region Drop magic functionality
	UFUNCTION(BlueprintCallable)
	virtual void DropMagic();

	UFUNCTION(NetMulticast, unreliable)
	void Multicast_HandleStandbyCosmetics(bool NewEnabled);

	#pragma endregion

	UFUNCTION(BlueprintCallable)
	void PrepareProjectileSpawn();

	UFUNCTION(Server, Reliable)
	void Server_SpawnProjectile(const FTransform& SpawnTransform);

	UFUNCTION(BlueprintCallable)
	void SpawnAOE(const FVector_NetQuantize& TargetLocation);

	//Line trace for item under the crosshairs
	bool TraceUnderCrosshairs(FHitResult& OutHitResult, FVector& OutHitLocation);
	void GetBeamEndLocation(const FVector& StartLocation, FHitResult& OutHitResult);

private:
	void SpawnIdleClass();
	TObjectPtr<AWOGBaseIdleMagic> IdleActor;

	void AddAbilityWidget(const int32& Key);
	UPROPERTY(Replicated)
	int32 AbilityKey = 1;

public:	

	UFUNCTION(BlueprintPure)
	FORCEINLINE FMagicDataTable GetMagicData() const { return MagicData; }
};
