// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "WOGRavenAIController.generated.h"

class AWOGRaven;
class AWOGSpline;
class ACPathVolume;
class UPawnMovementComponent;

/**
 * 
 */
UCLASS()
class WOG_API AWOGRavenAIController : public AAIController
{
	GENERATED_BODY()

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void StartRavenMovement();
	
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	TObjectPtr<AWOGRaven> Raven;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TObjectPtr<AWOGSpline> RavenSpline;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TObjectPtr<ACPathVolume> NavVolumeRef;

	UFUNCTION(BlueprintCallable)
	void FollowRavenSpline();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool bIsOnSpline = false;

private:

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FVector Destination = FVector();

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	FVector CurrentInputVector = FVector();

	void FindClosestPoint();

	TObjectPtr<UPawnMovementComponent> RavenMovement;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Debug")
	bool bDrawDebug = false;
};

