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

protected:
	virtual void BeginPlay() override;
	void SendAbilityLocalInput(const EWOGAbilityInputID InInputID);
	virtual void OnHealthAttributeChanged(const FOnAttributeChangeData& Data);
	virtual void OnMaxMovementSpeedAttributeChanged(const FOnAttributeChangeData& Data);
	UFUNCTION()
	virtual void OnStartAttack();
	UFUNCTION()
	virtual void OnAttackHit(FHitResult Hit, UPrimitiveComponent* WeaponMesh);
	virtual void ProcessHit(FHitResult Hit, UPrimitiveComponent* WeaponMesh) { /*TO-BE OVERRIDEN IN CHILDREN*/ };

	TArray<TObjectPtr<AActor>> HitActorsToIgnore;

	UPROPERTY(EditDefaultsOnly, Category = "Setup|Abilities and Effects")
	FCharacterAbilityData DefaultAbilitiesAndEffects;

	#pragma region Actor Components
	UPROPERTY()
	TObjectPtr<UWOGAbilitySystemComponent> AbilitySystemComponent;
	UPROPERTY()
	TObjectPtr<UWOGAttributeSetBase> AttributeSet;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAGR_CombatManager> CombatManager;

	#pragma endregion

	#pragma region  Character State
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
	bool bIsTargeting;

	UFUNCTION(NetMulticast, reliable)
	void Multicast_SetCharacterState(ECharacterState NewState, AController* InstigatedBy = nullptr);

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
	ECharacterState CharacterState;

	UFUNCTION(BlueprintCallable)
	void SetCharacterState(ECharacterState NewState, AController* InstigatedBy = nullptr);

	UFUNCTION(BlueprintCallable)
	virtual void HandleStateElimmed(AController* InstigatedBy = nullptr) { /*TO-BE OVERRIDEN IN CHILDREN*/ };
	virtual void HandleStateStaggered() { /*TO-BE OVERRIDEN IN CHILDREN*/ };

	#pragma endregion

	#pragma region Handle Elim
	FTimerHandle ElimTimer;
	FTimerDelegate ElimDelegate;

	UPROPERTY(EditDefaultsOnly)
	float ElimDelay = 6.f;
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

	bool IsHitFrontal(const float& AngleTolerance, const AActor* Victim, const AActor* Agressor);
	UFUNCTION(NetMulticast, reliable)
	void Multicast_HandleCosmeticHit(const ECosmeticHit& HitType, const FHitResult& Hit, const FVector& WeaponLocation, const AWOGBaseWeapon* InstigatorWeapon);

	#pragma region Cosmetic Hits
	//Handle cosmetic body hit
	virtual void HandleCosmeticBodyHit(const FHitResult& Hit, const FVector& WeaponLocation, const AWOGBaseWeapon* InstigatorWeapon);

	UFUNCTION(BlueprintPure)
	FName CalculateHitDirection(const FHitResult& Hit, const FVector& WeaponLocation);
	virtual void PlayHitReactMontage(FName Section);
	FVector LastHitDirection;

	//Handle cosmetic block
	virtual void HandleCosmeticBlock(const AWOGBaseWeapon* InstigatorWeapon);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup|Animations")
	TObjectPtr<class UAnimMontage> UnarmedHurtMontage;
	#pragma endregion

public:	
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void Elim(bool bPlayerLeftGame);

	bool HasMatchingGameplayTag(FGameplayTag TagToCheck) const;

	UFUNCTION(Server, reliable, BlueprintCallable)
	void Server_SetCharacterState(ECharacterState NewState, AController* InstigatedBy = nullptr);

	UFUNCTION(BlueprintPure)
	FORCEINLINE ECharacterState GetCharacterState() const { return CharacterState; }
	FORCEINLINE float GetSpeedRequiredForLeap() const { return SpeedRequiredForLeap; }
	FORCEINLINE UWOGAttributeSetBase* GetAttributeSetBase() const { return AttributeSet; }
	FORCEINLINE void SetDefaultAbilitiesAndEffects(const FCharacterAbilityData& Data) { DefaultAbilitiesAndEffects = Data; }
};
