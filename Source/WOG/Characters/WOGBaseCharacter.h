// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "WOG/Types/CharacterTypes.h"
#include "WOG/Interfaces/AttributesInterface.h"
#include "WOGBaseCharacter.generated.h"

UCLASS()
class WOG_API AWOGBaseCharacter : public ACharacter, public IAttributesInterface
{
	GENERATED_BODY()

public:
	AWOGBaseCharacter();
	friend class UWOGAttributesComponent;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UWOGAttributesComponent* Attributes;

	#pragma region  Character State
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
	bool bIsTargeting;

	UFUNCTION(NetMulticast, reliable)
	void Multicast_SetCharacterState(ECharacterState NewState, AController* InstigatedBy = nullptr);

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
	ECharacterState CharacterState;

	void SetCharacterState(ECharacterState NewState, AController* InstigatedBy = nullptr);

	virtual void HandleStateElimmed(AController* InstigatedBy = nullptr);
	virtual void HandleStateSprinting();
	virtual void HandleStateUnnoccupied();
	virtual void HandleStateDodging();
	virtual void HandleStateAttacking();

	#pragma endregion

	#pragma region Handle Elim
	FTimerHandle ElimTimer;
	FTimerDelegate ElimDelegate;

	UPROPERTY(EditDefaultsOnly)
	float ElimDelay = 6.f;
	#pragma endregion

	#pragma region Interfaces functions

	virtual void BroadcastHit_Implementation(AActor* AgressorActor, const FHitResult& Hit, const float& DamageToApply, AActor* InstigatorWeapon) override;
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

	UFUNCTION()
	virtual void ReceiveDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	virtual void Elim(bool bPlayerLeftGame);

	UFUNCTION(Server, reliable)
	void Server_SetCharacterState(ECharacterState NewState, AController* InstigatedBy = nullptr);

	UFUNCTION(BlueprintPure)
	FORCEINLINE ECharacterState GetCharacterState() const { return CharacterState; }
};
