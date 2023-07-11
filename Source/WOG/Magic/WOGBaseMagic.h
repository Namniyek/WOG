// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
#include "GameplayAbilitySpec.h"
#include "WOGBaseMagic.generated.h"

class UAnimMontage;
class USoundCue;
class ABasePlayerCharacter;
class UGameplayEffect;
class UAGR_ItemComponent;
class UAGR_InventoryManager;
class USphereComponent;
class UNiagaraSystem;

USTRUCT(BlueprintType)
struct FMagicDataTable : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Data")
	FName MagicName = FName("");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Data")
	bool bIsAttacker = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Data")
	FName LeftHandSocket = FName("");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Data")
	FName RighHandSocket = FName("");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Data")
	FGameplayTag MagicTag = FGameplayTag();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Data")
	FGameplayTag MagicPoseTag = FGameplayTag();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
	UAnimMontage* AttackMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
	UAnimMontage* EquipMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
	UAnimMontage* HurtMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float BaseDamage = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float DamageMultiplier = 0.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GAS")
	float StunDuration = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	USoundCue* CastSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	USoundCue* HitSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	TObjectPtr<UNiagaraSystem> IdleParticleSystem = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Data")
	TSubclassOf<UGameplayEffect> WeaponDamageEffect = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Data")
	TSubclassOf<UGameplayEffect> RangedWeaponEffect = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GAS")
	TArray<TSubclassOf<class UWOGGameplayAbilityBase>> Abilities = { nullptr };

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GAS")
	FGameplayTag RangedTag = FGameplayTag();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animations")
	float AnimationSpeed = 1.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GAS")
	TSubclassOf<AActor> RangedClass = nullptr;

	//TO-DO SFX particles for weapon trail && hit FX

};

UCLASS()
class WOG_API AWOGBaseMagic : public AActor
{
	GENERATED_BODY()
	
public:	
	AWOGBaseMagic();
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

	#pragma region MagicVariables
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName MagicName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FMagicDataTable MagicData;
	#pragma endregion

	#pragma region Item Functions
	UFUNCTION()
	void OnMagicOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UFUNCTION()
	void OnMagicPickedUp(UAGR_InventoryManager* Inventory);

	UFUNCTION()
	void OnMagicEquip(AActor* User, FName SlotName);

	UFUNCTION()
	void OnMagicUnequip(AActor* User, FName SlotName);

	UFUNCTION(NetMulticast, reliable)
	void Multicast_OnMagicEquip(AActor* User, FName SlotName);

	void AttachToHands();
	void AttachToBack();
	#pragma endregion

	#pragma region GAS
	bool GrantMagicAbilities(AActor* User);
	bool RemoveGrantedAbilities(AActor* User);

	TArray<FGameplayAbilitySpecHandle> GrantedAbilities;
	#pragma endregion

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<ABasePlayerCharacter> OwnerCharacter;

private:
	virtual void InitMagicData();


public:	

	UFUNCTION(BlueprintPure)
	FORCEINLINE FMagicDataTable GetMagicData() const { return MagicData; }

	UFUNCTION(BlueprintCallable)
	void SetOwnerCharacter(ABasePlayerCharacter* NewOwner);
	UFUNCTION(BlueprintPure)
	FORCEINLINE ABasePlayerCharacter* GetOwnerCharacter() const { return OwnerCharacter; }

};
