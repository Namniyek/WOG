// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WOGBaseIdleMagic.generated.h"

class USkeletalMeshComponent;
class UNiagaraComponent;

UCLASS()
class WOG_API AWOGBaseIdleMagic : public AActor
{
	GENERATED_BODY()
	
public:	
	AWOGBaseIdleMagic();

protected:
	virtual void BeginPlay() override;
	virtual void Destroyed() override;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_HandleInit(USkeletalMeshComponent* Mesh, const FName& RightHand, const FName& LeftHand, USoundCue* NewSound);

	#pragma region ActorComponents
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UNiagaraComponent> RightHandEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UNiagaraComponent> LeftHandEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAudioComponent> IdleSound;
	#pragma endregion

public:	

};
