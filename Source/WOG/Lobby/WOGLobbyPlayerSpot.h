// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WOGLobbyPlayerSpot.generated.h"

UCLASS()
class WOG_API AWOGLobbyPlayerSpot : public AActor
{
	GENERATED_BODY()
	
public:	
	AWOGLobbyPlayerSpot();

	UPROPERTY(EditDefaultsOnly)
	USceneComponent* Root;

	UPROPERTY(EditDefaultsOnly)
	USkeletalMeshComponent* PreviewMesh;

	UPROPERTY(EditDefaultsOnly)
	USceneComponent* CameraBodyTransform;

	UPROPERTY(EditDefaultsOnly)
	USceneComponent* CameraFaceTransform;

	UPROPERTY(EditDefaultsOnly)
	class UCameraComponent* Camera;

	/*
	*Temp position for vars. Move to private later. Maybe...
	*/

	UPROPERTY(BlueprintReadWrite)
	bool bIsSpotUsed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int TeamIndex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int UserIndex;

	UPROPERTY(BlueprintReadWrite)
	class AWOGLobbyPlayerController* OwnerPC;

protected:
	virtual void BeginPlay() override;

private:


public:	
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void IsOccupied();

	UFUNCTION(BlueprintCallable)
	void IsEmpty();

	UFUNCTION(BlueprintCallable)
	void ChangeCameraToFace();

	UFUNCTION(BlueprintCallable)
	void ChangeCameraToBody();

};
