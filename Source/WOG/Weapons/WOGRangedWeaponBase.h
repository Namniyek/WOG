// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WOGBaseWeapon.h"
#include "WOGRangedWeaponBase.generated.h"

class UProjectileMovementComponent;
class URotatingMovementComponent;
class UAGR_CombatManager;

UCLASS()
class WOG_API AWOGRangedWeaponBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWOGRangedWeaponBase();
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


	#pragma region WeaponVariables

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UStaticMeshComponent* Mesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName WeaponName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FWeaponDataTable WeaponData;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float RecallTime;

	UPROPERTY(BlueprintReadOnly)
	bool bHitFound;

#pragma endregion

	#pragma region ActorComponents

		UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged Weapon Components")
		TObjectPtr <USceneComponent> Root;

		UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged Weapon Components")
		TObjectPtr <URotatingMovementComponent> ThrowRotation;

		UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged Weapon Components")
		TObjectPtr <URotatingMovementComponent> RecallRotation;

		UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged Weapon Components")
		TObjectPtr <UProjectileMovementComponent> ProjectileComponent;

		UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		TObjectPtr<UAGR_CombatManager> CombatManager;

	#pragma endregion

	void InitWeapon();

	UFUNCTION()
	void OnAttackHit(FHitResult Hit, UPrimitiveComponent* HitMesh);

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<ABasePlayerCharacter> OwnerCharacter;

private:
	float TraceDelay;
	FTimerHandle TraceDelayTimer;

	UFUNCTION()
	void StartTrace();

	void DropWeapon(const FVector& ImpactNormal);

public:	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void RecallWeapon();

	UFUNCTION(BlueprintPure)
	FORCEINLINE ABasePlayerCharacter* GetOwnerCharacter() const { return OwnerCharacter; }

};
