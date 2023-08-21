// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Types/CharacterTypes.h"
#include "Interfaces/AttributesInterface.h"
#include "Abilities/GameplayAbility.h"
#include "AbilitySystemInterface.h"
#include "Data/AGRLibrary.h"
#include "WOGBaseCharacter.generated.h"

class UWOGAbilitySystemComponent;
class UWOGAttributeSetBase;
class UGameplayEffect;
class UAbilitySystemComponent;
class AWOGGameMode;
class UAGRAnimMasterComponent;
class UMaterialInterface;
class UMotionWarpingComponent;

UCLASS()
class WOG_API AWOGBaseCharacter : public ACharacter, public IAttributesInterface, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AWOGBaseCharacter();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PossessedBy(AController* NewController) override;

	bool ApplyGameplayEffectToSelf(TSubclassOf<UGameplayEffect> Effect, FGameplayEffectContextHandle InEffectContext);
	
	void GiveDefaultAbilities();
	void ApplyDefaultEffects();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UMaterialInterface* Material;

	UFUNCTION(BlueprintCallable)
	virtual void ProcessMagicHit(const FHitResult& Hit, const struct FMagicDataTable& MagicData) { /*TO-BE OVERRIDEN IN CHILDREN*/ };

protected:
	virtual void BeginPlay() override;
	void SendAbilityLocalInput(const EWOGAbilityInputID InInputID);
	virtual void OnHealthAttributeChanged(const FOnAttributeChangeData& Data);
	virtual void OnMaxMovementSpeedAttributeChanged(const FOnAttributeChangeData& Data);
	virtual void OnGameplayEffectAppliedToSelf(UAbilitySystemComponent* Source, const FGameplayEffectSpec& Spec, FActiveGameplayEffectHandle Handle);
	UFUNCTION()
	virtual void OnStartAttack();
	UFUNCTION()
	virtual void OnAttackHit(FHitResult Hit, UPrimitiveComponent* WeaponMesh);
	virtual void ProcessHit(FHitResult Hit, UPrimitiveComponent* WeaponMesh) { /*TO-BE OVERRIDEN IN CHILDREN*/ };

	
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_SetCharacterFrozen(bool bIsFrozen);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetCharacterFrozen(bool bIsFrozen);

	UFUNCTION(BlueprintNativeEvent)
	void SetCharacterFrozen(bool bIsFrozen);

	virtual void ToggleStrafeMovement(bool bIsStrafe);

	TArray<TObjectPtr<AActor>> HitActorsToIgnore;

	UPROPERTY(EditDefaultsOnly, Category = "Setup|Abilities and Effects")
	FCharacterAbilityData DefaultAbilitiesAndEffects;

	UPROPERTY(VisibleAnywhere, Category = "Setup|Character Data")
	FCharacterData CharacterData;

	#pragma region Actor Components
	UPROPERTY()
	TObjectPtr<UWOGAbilitySystemComponent> AbilitySystemComponent;
	UPROPERTY()
	TObjectPtr<UWOGAttributeSetBase> AttributeSet;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAGR_CombatManager> CombatManager;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAGRAnimMasterComponent> AnimManager;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UMotionWarpingComponent> MotionWarping;

	#pragma endregion

	#pragma region Handle Elim
	FTimerHandle ElimTimer;
	FTimerDelegate ElimDelegate;

	UPROPERTY(EditDefaultsOnly)
	float ElimDelay = 6.f;

	UFUNCTION(BlueprintCallable)
	virtual void HandleStateElimmed(AController* InstigatedBy = nullptr) { /*TO-BE OVERRIDEN IN CHILDREN*/ };

	#pragma endregion

	#pragma region Interfaces functions

	virtual void BroadcastHit_Implementation(AActor* AgressorActor, const FHitResult& Hit, const float& DamageToApply, AActor* InstigatorWeapon) override {/*To be overriden in Children*/};

public:
	UFUNCTION(BlueprintPure)
	UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	#pragma endregion

	#pragma region Animation Variables

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup|Animations")
	float SpeedRequiredForLeap;

	#pragma endregion

	UPROPERTY()
	TObjectPtr<AWOGGameMode> WOGGameMode;

	#pragma region Cosmetic Hits
	/*
	**Calculate if the hit is frontal
	**Will check Agressor param first, and if not valid will fallback to vector
	*/
	UFUNCTION(BlueprintPure)
	bool IsHitFrontal(const float& AngleTolerance, const AActor * Victim, const FVector& Location, const AActor* Agressor = nullptr);

	/*
	**Calcualate Hit direction. Returns name of the direction
	*/
	UFUNCTION(BlueprintPure)
	FName CalculateHitDirection(const FVector& WeaponLocation);
	FVector LastHitDirection;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	FHitResult LastHitResult;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup|Animations")
	TObjectPtr<class UAnimMontage> UnarmedHurtMontage;
	#pragma endregion

	UPROPERTY(BlueprintReadWrite)
	bool bSecondaryButtonPressed = false;

public:	
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void Elim(bool bPlayerLeftGame) { /*To be overriden in Children*/ };

	bool HasMatchingGameplayTag(FGameplayTag TagToCheck) const;

	FORCEINLINE float GetSpeedRequiredForLeap() const { return SpeedRequiredForLeap; }
	FORCEINLINE UWOGAttributeSetBase* GetAttributeSetBase() const { return AttributeSet; }
	FORCEINLINE void SetDefaultAbilitiesAndEffects(const FCharacterAbilityData& Data) { DefaultAbilitiesAndEffects = Data; }
	FORCEINLINE FCharacterData GetCharacterData() const { return CharacterData; }
	FORCEINLINE TObjectPtr<UMotionWarpingComponent> GetMotionWarpingComponent() const { return MotionWarping; }
};
