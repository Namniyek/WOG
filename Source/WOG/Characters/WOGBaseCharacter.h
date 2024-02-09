// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Types/CharacterTypes.h"
#include "Data/WOGDataTypes.h"
#include "Interfaces/AttributesInterface.h"
#include "Abilities/GameplayAbility.h"
#include "AbilitySystemInterface.h"
#include "Data/AGRLibrary.h"
#include "Interfaces/TargetInterface.h"
#include "Kismet/KismetMathLibrary.h"
#include "Blueprint/UserWidget.h"
#include "Components/TimelineComponent.h"
#include "WOGBaseCharacter.generated.h"

class UWOGAbilitySystemComponent;
class UWOGAttributeSetBase;
class UGameplayEffect;
class UAbilitySystemComponent;
class AWOGGameMode;
class UAGRAnimMasterComponent;
class UMaterialInterface;
class UMotionWarpingComponent;
class UWOGHoldProgressBar;
class UWidgetComponent;
class UWOGCharacterWidgetContainer;
class UAnimMontage;

UCLASS()
class WOG_API AWOGBaseCharacter : public ACharacter, public IAttributesInterface, public IAbilitySystemInterface, public ITargetInterface
{
	GENERATED_BODY()

public:
	AWOGBaseCharacter();

	virtual void PossessedBy(AController* NewController) override;
	virtual void Tick(float DeltaTime) override;
	virtual void Elim(bool bPlayerLeftGame) { /*To be overriden in Children*/ };

protected:
	virtual void BeginPlay() override;

	UPROPERTY()
	TObjectPtr<AWOGGameMode> WOGGameMode;

	#pragma region GAS functions
public:
	bool ApplyGameplayEffectToSelf(TSubclassOf<UGameplayEffect> Effect, FGameplayEffectContextHandle InEffectContext, float Duration = 1.f);

	void GiveDefaultAbilities();
	void ApplyDefaultEffects();

	bool HasMatchingGameplayTag(FGameplayTag TagToCheck) const;

	UFUNCTION(BlueprintPure)
	UAbilitySystemComponent* GetAbilitySystemComponent() const override;

protected:
	void SendAbilityLocalInput(const EWOGAbilityInputID InInputID);
	virtual void OnHealthAttributeChanged(const FOnAttributeChangeData& Data);
	void GiveDeathResources(AActor* InActor);
	virtual void OnStaminaAttributeChanged(const FOnAttributeChangeData& Data);
	virtual void OnMaxMovementSpeedAttributeChanged(const FOnAttributeChangeData& Data);
	virtual void OnGameplayEffectAppliedToSelf(UAbilitySystemComponent* Source, const FGameplayEffectSpec& Spec, FActiveGameplayEffectHandle Handle);
	#pragma endregion
	
	#pragma region TimeOfDay
	UFUNCTION()
	void TimeOfDayChanged(ETimeOfDay TOD);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_CurrentTOD)
	ETimeOfDay CurrentTOD;

	UFUNCTION()
	void OnRep_CurrentTOD();

	virtual void HandleTODChange() {/*TO BE OVERRIDEN IN CHILDREN*/ };
	#pragma endregion

	#pragma region Freeze functionality
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_SetCharacterFrozen(bool bIsFrozen);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetCharacterFrozen(bool bIsFrozen);

	UFUNCTION(BlueprintNativeEvent)
	void SetCharacterFrozen(bool bIsFrozen);
	#pragma endregion

	#pragma region Setup
	UPROPERTY(EditDefaultsOnly, Category = "Setup|Abilities and Effects")
	FCharacterAbilityData DefaultAbilitiesAndEffects;

	UPROPERTY(VisibleAnywhere, Category = "Setup|Character Data")
	FCharacterData CharacterData;

	UPROPERTY(EditDefaultsOnly, Category = "Setup|Character Data")
	TMap<TSubclassOf<AActor>, int32> DeathResourceMap;

	UPROPERTY(Replicated, VisibleAnywhere, Category = "Setup|Combat")
	int32 AvailableAttackTokens;

	UPROPERTY(EditDefaultsOnly, Category = "Setup|Combat")
	int32 MaxAttackTokens;
	#pragma endregion

	#pragma region Actor Components
	UPROPERTY()
	TObjectPtr<UWOGAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UWOGAttributeSetBase> AttributeSet;

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

	#pragma region Ragdoll

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_ToRagdoll(const float& Radius, const float& Strength, const FVector_NetQuantize& Origin);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ToRagdoll(const float& Radius, const float& Strength, const FVector_NetQuantize& Origin);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_ToAnimation();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ToAnimation();

	UFUNCTION(BlueprintCallable)
	void ToAnimationSecondStage();
	UFUNCTION(BlueprintCallable)
	void ToAnimationThirdStage();

	UFUNCTION(BlueprintCallable)
	void ToAnimationFinal();

	void FindCharacterOrientation();
	void SetCapsuleOrientation();
	void UpdateCapsuleLocation();

	void InitPhysics();

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	bool bIsLayingOnBack;
	UPROPERTY(Replicated, BlueprintReadOnly, VisibleAnywhere)
	bool bIsRagdolling;

	FVector MeshLocation;
	FVector TargetGroundLocation;
	FVector PelvisOffset;

	FVectorSpringState SpringState;
	#pragma endregion

	#pragma region DissolveEffect

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UMaterialInterface* Material;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UMaterialInstanceDynamic* CharacterMI;

	UPROPERTY(VisibleAnywhere)
	UTimelineComponent* DissolveTimeline;
	FOnTimelineFloat DissolveTrack;
	FOnTimelineEvent DissolveTimelineFinished;

	UPROPERTY(EditDefaultsOnly, Category = "Setup|Dissolve")
	TObjectPtr<UCurveFloat> DissolveCurve;

	UPROPERTY(EditDefaultsOnly, Category = "Setup|Dissolve")
	FLinearColor DissolveColor;

	UFUNCTION()
	void UpdateDissolveMaterial(float DissolveValue);

	UFUNCTION()
	void OnDissolveTimelineFinished();

	UFUNCTION(NetMulticast, reliable)
	void Multicast_StartDissolve(bool bIsReversed = false);

	void StartDissolve(bool bIsReversed = false);
	#pragma endregion

	#pragma region Animation
	virtual void ToggleStrafeMovement(bool bIsStrafe);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup|Animations")
	TObjectPtr<UAnimMontage> UnarmedHurtMontage;
	#pragma endregion

	#pragma region Cosmetic Hits
	/*
	**Calculate if the hit is frontal
	**Will check Agressor param first, and if not valid will fallback to vector
	*/
	UFUNCTION(BlueprintPure)
	bool IsHitFrontal(const float& AngleTolerance, const AActor* Victim, const FVector& Location, const AActor* Agressor = nullptr);

	/*
	**Calcualate Hit direction. Returns name of the direction
	*/
	UFUNCTION(BlueprintPure)
	FName CalculateHitDirection(const FVector& WeaponLocation);

	FVector LastHitDirection;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	FHitResult LastHitResult;
	#pragma endregion

	#pragma region Interface Functions

	bool IsTargetable_Implementation(AActor* TargeterActor) const;

	#pragma endregion

public:	
	FORCEINLINE UWOGAttributeSetBase* GetAttributeSetBase() const { return AttributeSet; }
	FORCEINLINE void SetDefaultAbilitiesAndEffects(const FCharacterAbilityData& Data) { DefaultAbilitiesAndEffects = Data; }
	FORCEINLINE FCharacterData GetCharacterData() const { return CharacterData; }
	FORCEINLINE TObjectPtr<UMotionWarpingComponent> GetMotionWarpingComponent() const { return MotionWarping; }
	UFUNCTION(BlueprintPure)
	FORCEINLINE bool GetIsRagdolling() const { return bIsRagdolling; }
	UFUNCTION(BlueprintPure)
	FORCEINLINE bool GetIsLayingOnBack() const { return bIsLayingOnBack; }

	#pragma region Handle Combat
	virtual void ProcessHit(FHitResult Hit, UPrimitiveComponent* WeaponMesh) {/*To be overriden in children*/ };
	virtual void ProcessMagicHit(const FHitResult& Hit, const struct FMagicDataTable& MagicData) {/*To be overriden in children*/ };
	#pragma endregion
};
