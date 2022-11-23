// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BasePlayerCharacter.generated.h"

UCLASS()
class WOG_API ABasePlayerCharacter : public ACharacter
{
	GENERATED_BODY()
public:
	// Sets default values for this character's properties
	ABasePlayerCharacter();
	
	/*
	** Variables
	*/

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Input)
	float TurnRateGamepad; //Base turn rate, in deg/sec. Other scaling may affect final turn rate. */

	/*
	** Functions
	*/

	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:

	/*
	** Variables
	*/

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	//Meshes
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Meshes)
	class USkeletalMeshComponent* MeshTorso;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Meshes)
	USkeletalMeshComponent* MeshHips;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Meshes)
	USkeletalMeshComponent* MeshLegLeft;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Meshes)
	USkeletalMeshComponent* MeshLegRight;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Meshes)
	USkeletalMeshComponent* MeshArmUpperLeft;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Meshes)
	USkeletalMeshComponent* MeshArmUpperRight;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Meshes)
	USkeletalMeshComponent* MeshArmLowerLeft;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Meshes)
	USkeletalMeshComponent* MeshArmLowerRight;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Meshes)
	USkeletalMeshComponent* MeshHandLeft;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Meshes)
	USkeletalMeshComponent* MeshHandRight;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Meshes)
	USkeletalMeshComponent* MeshEyebrows;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Meshes)
	USkeletalMeshComponent* MeshEars;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Meshes)
	USkeletalMeshComponent* MeshHair;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Meshes)
	USkeletalMeshComponent* MeshBeard;

	/*
	** Functions
	*/

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/*
	**Player Input Section 
	*/

	void MoveForward(float Value);

	void MoveRight(float Value);

	void TurnAtRate(float Rate); //@param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate

	void LookUpAtRate(float Rate); //@param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


public:
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }

};
