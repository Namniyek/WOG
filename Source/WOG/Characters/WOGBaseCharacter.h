// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Types/CharacterTypes.h"
#include "Interfaces/AttributesInterface.h"
#include "Abilities/GameplayAbility.h"
#include "AbilitySystemInterface.h"
#include "WOGBaseCharacter.generated.h"

class UWOGAbilitySystemComponent;
class UWOGAttributeSetBase;
class UGameplayEffect;
class UAbilitySystemComponent;

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

	UPROPERTY(VisibleAnywhere)
	FCharacterAbilityData DefaultAbilitiesAndEffects;

	UPROPERTY()
	TObjectPtr<UWOGAbilitySystemComponent> AbilitySystemComponent;
	UPROPERTY()
	TObjectPtr<UWOGAttributeSetBase> AttributeSet;

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
	virtual void HandleStateSprinting() { /*TO-BE OVERRIDEN IN CHILDREN*/ };
	virtual void HandleStateUnnoccupied() { /*TO-BE OVERRIDEN IN CHILDREN*/ };
	virtual void HandleStateDodging() { /*TO-BE OVERRIDEN IN CHILDREN*/ };
	virtual void HandleStateAttacking() { /*TO-BE OVERRIDEN IN CHILDREN*/ };
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
	#pragma endregion

	#pragma region Animation Variables

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float SpeedRequiredForLeap;

	#pragma endregion

	UPROPERTY()
	TObjectPtr<class AWOGGameMode> WOGGameMode;

	bool IsHitFrontal(const float& AngleTolerance, const AActor* Victim, const AActor* Agressor);
	UFUNCTION(NetMulticast, reliable)
	void Multicast_HandleCosmeticHit(const ECosmeticHit& HitType, const FHitResult& Hit, const FVector& WeaponLocation, const AWOGBaseWeapon* InstigatorWeapon);

	#pragma region Cosmetic Hits
	//Handle cosmetic body hit
	virtual void HandleCosmeticBodyHit(const FHitResult& Hit, const FVector& WeaponLocation, const AWOGBaseWeapon* InstigatorWeapon);
	FName CalculateHitDirection(const FHitResult& Hit, const FVector& WeaponLocation);
	virtual void PlayHitReactMontage(FName Section);
	FVector LastHitDirection;

	//Handle cosmetic block
	virtual void HandleCosmeticBlock(const AWOGBaseWeapon* InstigatorWeapon);

	//Handle cosmetic weapon clash
	virtual void HandleCosmeticWeaponClash();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<class UAnimMontage> UnarmedHurtMontage;
	#pragma endregion

public:	
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void Elim(bool bPlayerLeftGame);

	UFUNCTION(BlueprintPure)
	UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UFUNCTION(Server, reliable, BlueprintCallable)
	void Server_SetCharacterState(ECharacterState NewState, AController* InstigatedBy = nullptr);

	UFUNCTION(BlueprintPure)
	FORCEINLINE ECharacterState GetCharacterState() const { return CharacterState; }
	FORCEINLINE float GetSpeedRequiredForLeap() const { return SpeedRequiredForLeap; }
	FORCEINLINE UWOGAttributeSetBase* GetAttributeSetBase() const { return AttributeSet; }
	FORCEINLINE void SetDefaultAbilitiesAndEffects(const FCharacterAbilityData& Data) { DefaultAbilitiesAndEffects = Data; }
};
