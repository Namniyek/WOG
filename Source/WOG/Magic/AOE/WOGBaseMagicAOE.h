// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Magic/WOGBaseMagic.h"
#include "WOGBaseMagicAOE.generated.h"

class USphereComponent;
class UNiagaraComponent;

UCLASS()
class WOG_API AWOGBaseMagicAOE : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWOGBaseMagicAOE();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	void Init();

	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	FTimerHandle InitDelayTimer;

	UPROPERTY(EditDefaultsOnly)
	float InitDelay;

	#pragma region ActorComponents
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<USphereComponent> OverlapSphere;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UNiagaraComponent> AOEEffect;
	#pragma endregion

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated)
	FMagicDataTable MagicData;

public:
	FORCEINLINE void SetMagicData(const FMagicDataTable& NewData) { MagicData = NewData; }

	UFUNCTION(BlueprintPure)
	FORCEINLINE FMagicDataTable GetMagicData() const { return MagicData; }

};
