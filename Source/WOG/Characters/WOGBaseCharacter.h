// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "Abilities/GameplayAbility.h"
#include "Components/TimelineComponent.h"
#include "Data/AGRLibrary.h"
#include "Data/WOGDataTypes.h"
#include "GameFramework/Character.h"
#include "Interfaces/AttributesInterface.h"
#include "Interfaces/TargetInterface.h"
#include "Kismet/KismetMathLibrary.h"
#include "Types/CharacterTypes.h"
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
class AWOGPlayerController;
class UInputMappingContext;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnAttributeChangedDelegate, FGameplayAttribute, ChangedAttribute, float, NewValue, float, MaxValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCharacterDeadDelegate, AWOGBaseCharacter*, DeadCharacter);


UCLASS()
class WOG_API AWOGBaseCharacter : public ACharacter, public IAttributesInterface, public IAbilitySystemInterface, public ITargetInterface
{
	GENERATED_BODY()

public:
	AWOGBaseCharacter();

	virtual void PossessedBy(AController* NewController) override;
	virtual void Tick(float DeltaTime) override;
	UFUNCTION(BlueprintCallable)
	virtual void Elim(bool bPlayerLeftGame) { /*To be overriden in Children*/ };

	#pragma region Handle Combat
	//Line trace for item under the crosshair
	bool TraceUnderCrosshair(FHitResult& OutHitResult, FVector& OutHitLocation) const;
	UFUNCTION(BlueprintCallable)
	void GetBeamEndLocation(const FVector& StartLocation, FHitResult& OutHitResult);
	#pragma endregion 

protected:
	virtual void BeginPlay() override;

	UPROPERTY()
	TObjectPtr<AWOGGameMode> WOGGameMode;

	UPROPERTY(Replicated)
	TObjectPtr<AWOGPlayerController> OwnerPC = nullptr;

	UPROPERTY(BlueprintReadWrite)
	bool bSecondaryButtonPressed = false;

	#pragma region GAS functions
public:
	bool ApplyGameplayEffectToSelf(const TSubclassOf<UGameplayEffect>& Effect, const FGameplayEffectContextHandle& InEffectContext, float Duration = 1.f) const;

	void GiveDefaultAbilities();
	void ApplyDefaultEffects();
	virtual void GrantDefaultInventoryItems();

	UFUNCTION(BlueprintPure)
	bool HasMatchingGameplayTag(FGameplayTag TagToCheck) const;

	UFUNCTION(BlueprintPure)
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UPROPERTY(BlueprintAssignable)
	FOnAttributeChangedDelegate OnAttributeChangedDelegate;
	
	UPROPERTY(BlueprintAssignable)
	FOnCharacterDeadDelegate OnCharacterDeadDelegate;

	//TO-DO find another way to pass data to abilities. I don't like this
	UPROPERTY(BlueprintReadWrite)
	FGameplayEventData AbilityActivationPayload;

protected:
	void SendAbilityLocalInput(const EWOGAbilityInputID InInputID) const;
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
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup|Input|Mapping Contexts", meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* MainMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Setup|Abilities and Effects")
	FCharacterAbilityData DefaultAbilitiesAndEffects;

	UPROPERTY(Replicated, VisibleAnywhere, Category = "Setup|Character Data")
	FCharacterData CharacterData;

	UPROPERTY(EditDefaultsOnly, Category = "Setup|Character Data")
	TMap<TSubclassOf<AActor>, int32> DeathResourceMap;

	UPROPERTY(Replicated, VisibleAnywhere, Category = "Setup|Combat")
	int32 AvailableAttackTokens;

	UPROPERTY(EditDefaultsOnly, Category = "Setup|Combat")
	int32 MaxAttackTokens;

	UFUNCTION(NetMulticast, reliable)
	void Multicast_OnPossessed();

	virtual void ReplicatedOnPossessEvent();
	void SetupMappingContext() const;
	#pragma endregion

	#pragma region Actor Components
	UPROPERTY()
	TObjectPtr<UWOGAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UWOGAttributeSetBase> AttributeSet;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAGRAnimMasterComponent> AnimManager;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UMotionWarpingComponent> MotionWarping;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<USceneComponent> OverheadWidgetLocation;
	#pragma endregion

	#pragma region Handle Elim
	FTimerHandle ElimTimer;
	FTimerDelegate ElimDelegate;

	UPROPERTY(EditDefaultsOnly)
	float ElimDelay = 6.f;

	UFUNCTION(BlueprintCallable)
	virtual void HandleStateElimmed(AController* InstigatedBy = nullptr) { /*TO-BE OVERRIDEN IN CHILDREN*/ };

	void SetCapsulePawnCollision(const bool& bEnable) const;

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
	void ToAnimationFinal() const;

	void FindCharacterOrientation();
	void SetCapsuleOrientation() const;
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
	#pragma endregion

	#pragma region Cosmetic Hits
	/*
	**Calculate if the hit is frontal
	**Will check Aggressor param first, and if not valid will fall back to vector
	*/
	UFUNCTION(BlueprintPure)
	bool IsHitFrontal(const float& AngleTolerance, const AActor* Victim, const FVector& Location, const AActor* Aggressor = nullptr) const;

	/*
	**Calculate Hit direction. Returns name of the direction
	*/
	UFUNCTION(BlueprintPure)
	FName CalculateHitDirection(const FVector& WeaponLocation);

	FVector LastHitDirection;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	FHitResult LastHitResult;
	#pragma endregion

	#pragma region Interface Functions

	virtual bool IsTargetable_Implementation(AActor* TargeterActor) const override;
	virtual bool CanBePossessed_Implementation() const override;

	#pragma endregion

public:	
	FORCEINLINE UWOGAttributeSetBase* GetAttributeSetBase() const { return AttributeSet; }
	FORCEINLINE void SetDefaultAbilitiesAndEffects(const FCharacterAbilityData& Data) { DefaultAbilitiesAndEffects = Data; }
	UFUNCTION(BlueprintPure)
	FORCEINLINE FCharacterData GetCharacterData() const { return CharacterData; }
	FORCEINLINE void SetCharacterData(const FCharacterData& NewCharacterData) { CharacterData = NewCharacterData; }
	FORCEINLINE TObjectPtr<UMotionWarpingComponent> GetMotionWarpingComponent() const { return MotionWarping; }
	UFUNCTION(BlueprintPure)
	FORCEINLINE bool GetIsRagdolling() const { return bIsRagdolling; }
	UFUNCTION(BlueprintPure)
	FORCEINLINE bool GetIsLayingOnBack() const { return bIsLayingOnBack; }

	FORCEINLINE void SetOwnerPC(AWOGPlayerController* NewPC) { OwnerPC = NewPC; }
	UFUNCTION(BlueprintPure)
	FORCEINLINE AWOGPlayerController* GetOwnerPC() const { return OwnerPC; }

	#pragma region Handle Combat
	UFUNCTION(BlueprintCallable)
	virtual void ProcessHit(FHitResult Hit, UPrimitiveComponent* WeaponMesh) {/*To be overriden in children*/ };
	UFUNCTION(BlueprintCallable)
	virtual void ProcessMagicHit(const FHitResult& Hit, const struct FMagicDataTable& MagicData) {/*To be overriden in children*/ };
	UFUNCTION(BlueprintCallable)
	virtual void ProcessRangedHit(const FHitResult& Hit, const float& DamageToApply, AActor* AggressorWeapon) {/*To be overriden in children*/ };

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
	bool bComboWindowOpen;
	#pragma endregion
};
