// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "WOG/Data/PlayerProfileSaveGame.h"
#include "BasePlayerCharacter.generated.h"

UCLASS()
class WOG_API ABasePlayerCharacter : public ACharacter
{
	GENERATED_BODY()

	virtual void OnConstruction(const FTransform& Transform) override;

public:
	// Sets default values for this character's properties
	ABasePlayerCharacter();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	/*
	** Variables
	*/

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Input)
	float TurnRateGamepad; //Base turn rate, in deg/sec. Other scaling may affect final turn rate. */

	UPROPERTY(ReplicatedUsing = OnRep_PlayerProfile, EditDefaultsOnly, BlueprintReadWrite)
	FPlayerData PlayerProfile;

	UPROPERTY(EditAnywhere)
	UMaterialInterface* Material;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UMaterialInstanceDynamic* CharacterMI;

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
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	USkeletalMeshComponent* Head;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	USkeletalMeshComponent* ArmUpperLeft;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	USkeletalMeshComponent* ArmUpperRight;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	USkeletalMeshComponent* ArmLowerLeft;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	USkeletalMeshComponent* ArmLowerRight;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	USkeletalMeshComponent* HandLeft;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	USkeletalMeshComponent* HandRight;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	USkeletalMeshComponent* Torso;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	USkeletalMeshComponent* Hips;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	USkeletalMeshComponent* LegLeft;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	USkeletalMeshComponent* LegRight;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	USkeletalMeshComponent* Beard;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	USkeletalMeshComponent* Eyebrows;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	USkeletalMeshComponent* Hair;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	USkeletalMeshComponent* Helmet;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	USkeletalMeshComponent* Ears;

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

	UFUNCTION()
	void OnRep_PlayerProfile();

	UFUNCTION(BlueprintImplementableEvent)
	void UpdatePlayerProfile(FPlayerData NewPlayerProfile);


public:
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	UFUNCTION(Server, reliable, BlueprintCallable)
	void Server_SetPlayerProfile(FPlayerData NewPlayerProfile);

};
