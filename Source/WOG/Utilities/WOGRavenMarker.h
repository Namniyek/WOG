// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WOGRavenMarker.generated.h"

class USphereComponent;
class URotatingMovementComponent;
class UNiagaraSystem;

UCLASS()
class WOG_API AWOGRavenMarker : public AActor
{
	GENERATED_BODY()
	
public:	
	AWOGRavenMarker();

protected:
	virtual void BeginPlay() override;
	virtual void Destroyed() override;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<USphereComponent> SphereComponent;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<URotatingMovementComponent> RotatingComponent;

	UPROPERTY(EditDefaultsOnly)
	double InitDelay;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UNiagaraSystem> DespawnEffect;

private:
	void Init();

	UFUNCTION()
	void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

public:	


};
