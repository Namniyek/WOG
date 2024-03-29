// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Magic/WOGBaseMagic.h"
#include "WOGBaseMagicProjectile.generated.h"

class USphereComponent;
class UNiagaraComponent;
class UProjectileMovementComponent;
class UAudioComponent;

UCLASS()
class WOG_API AWOGBaseMagicProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	AWOGBaseMagicProjectile();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);


	#pragma region ActorComponents
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<USphereComponent> OverlapSphere;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UNiagaraComponent> ProjectileEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAudioComponent> ProjectileSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;
	#pragma endregion

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated)
	FMagicDataTable MagicData;

	

public:	
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	FORCEINLINE void SetMagicData(const FMagicDataTable& NewData) { MagicData = NewData; }

	UFUNCTION(BlueprintPure)
	FORCEINLINE FMagicDataTable GetMagicData() const { return MagicData; }

	UPROPERTY(BlueprintReadWrite, meta = (ExposeOnSpawn = "true"))
	TObjectPtr<AActor> Target = nullptr;

	UFUNCTION(BlueprintPure)
	FORCEINLINE UProjectileMovementComponent* GetProjectileMovementComponent() const { return ProjectileMovement; }

};
