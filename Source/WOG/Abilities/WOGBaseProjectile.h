// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WOGBaseProjectile.generated.h"

UCLASS()
class WOG_API AWOGBaseProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	AWOGBaseProjectile();

protected:
	virtual void BeginPlay() override;

	/** ProjectileMovementComponent */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default")
	TObjectPtr<class UProjectileMovementComponent> ProjectileMovement;

	/** Sphere Collision */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default")
	TObjectPtr< class USphereComponent> Sphere;

	/** Projectile VFX */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default")
	TObjectPtr<class UNiagaraComponent> ProjectileParticles;

	/** Base damage for the projectile */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Default")
	float Damage;


public:	


};
