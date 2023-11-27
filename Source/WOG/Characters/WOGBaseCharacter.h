// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Types/CharacterTypes.h"
#include "Interfaces/AttributesInterface.h"
#include "Abilities/GameplayAbility.h"
#include "AbilitySystemInterface.h"
#include "Data/AGRLibrary.h"
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

UCLASS()
class WOG_API AWOGBaseCharacter : public ACharacter, public IAttributesInterface, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AWOGBaseCharacter();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PossessedBy(AController* NewController) override;

	bool ApplyGameplayEffectToSelf(TSubclassOf<UGameplayEffect> Effect, FGameplayEffectContextHandle InEffectContext, float Duration = 1.f);
	
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
	void GiveDeathResources(AActor* InActor);
	virtual void OnStaminaAttributeChanged(const FOnAttributeChangeData& Data);
	virtual void OnMaxMovementSpeedAttributeChanged(const FOnAttributeChangeData& Data);
	virtual void OnGameplayEffectAppliedToSelf(UAbilitySystemComponent* Source, const FGameplayEffectSpec& Spec, FActiveGameplayEffectHandle Handle);
	UFUNCTION()
	virtual void OnStartAttack();
	UFUNCTION()
	virtual void OnAttackHit(FHitResult Hit, UPrimitiveComponent* WeaponMesh);
	virtual void ProcessHit(FHitResult Hit, UPrimitiveComponent* WeaponMesh) { /*TO-BE OVERRIDEN IN CHILDREN*/ };

	
	#pragma region TimeOfDay
	UFUNCTION()
	void TimeOfDayChanged(ETimeOfDay TOD);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_CurrentTOD)
	ETimeOfDay CurrentTOD;

	UFUNCTION()
	void OnRep_CurrentTOD();

	virtual void HandleTODChange() {/*TO BE OVERRIDEN IN CHILDREN*/ };
	#pragma endregion

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

	UPROPERTY(EditDefaultsOnly, Category = "Setup|Character Data")
	TMap<TSubclassOf<AActor>, int32> DeathResourceMap;

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

	#pragma region UI
	UFUNCTION()
	void AddHoldProgressBar();
	UFUNCTION()
	void RemoveHoldProgressBarWidget();

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UWidgetComponent> StaminaWidgetContainer;
	#pragma endregion

	#pragma region DissolveEffect
	UPROPERTY(VisibleAnywhere)
	UTimelineComponent* DissolveTimeline;
	FOnTimelineFloat DissolveTrack;
	FOnTimelineEvent DissolveTimelineFinished;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UCurveFloat> DissolveCurve;

	UPROPERTY(EditDefaultsOnly)
	FLinearColor DissolveColor;

	UFUNCTION()
	void UpdateDissolveMaterial(float DissolveValue);

	UFUNCTION()
	void OnDissolveTimelineFinished();

	UFUNCTION(NetMulticast, reliable)
	void Multicast_StartDissolve(bool bIsReversed = false);

	UFUNCTION()
	void FinishTeleportCharacter(const FTransform& Destination);

public:
	UFUNCTION(BlueprintCallable, Server, reliable)
	void Server_StartTeleportCharacter(const FTransform& Destination);
protected:
	#pragma endregion

	#pragma region Material variables

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UMaterialInstanceDynamic* CharacterMI;

	#pragma endregion

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
	UFUNCTION(BlueprintPure)
	FORCEINLINE bool GetIsRagdolling() const { return bIsRagdolling; }
	UFUNCTION(BlueprintPure)
	FORCEINLINE bool GetIsLayingOnBack() const { return bIsLayingOnBack; }
	
	//Can return nullptr
	UWOGCharacterWidgetContainer* GetStaminaWidgetContainer() const;
};
