// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/WOGBaseCharacter.h"
#include "Interfaces/SpawnInterface.h"
#include "Interfaces/TargetInterface.h"
#include "InputActionValue.h"
#include "WOGRaven.generated.h"

/**
 * 
 */
class UInputAction;
class USpringArmComponent;
class UCameraComponent;
class AWOGSpline;
class AWOGRavenMarker;

UCLASS()
class WOG_API AWOGRaven : public AWOGBaseCharacter, public ISpawnInterface
{
	GENERATED_BODY()
	
public:
	AWOGRaven();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;

	#pragma region Player Input Variables
	/*
	** MappingContexts
	**/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Mapping Contexts", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputMappingContext> RavenMappingContext;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* PrimaryLightAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SecondaryAction;

	#pragma endregion

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup")
	TObjectPtr<AWOGSpline> AssignedSplinePath;

	#pragma region Components variables
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	#pragma endregion

	#pragma region Player Input Functions

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** Called for movement input */
	void MoveActionPressed(const FInputActionValue& Value);

	/** Called for looking input */
	void LookActionPressed(const FInputActionValue& Value);

	void PrimaryActionTriggered(const FInputActionValue& Value);
	void SecondaryActionTriggered(const FInputActionValue& Value);

	#pragma endregion

	virtual void PossessedBy(AController* NewController) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<AWOGRavenMarker> MarkerClass;

	virtual void UnpossessMinion_Implementation();

	bool CanBePossessed_Implementation() const;

private:

	UFUNCTION(Server, reliable)
	void Server_SpawnMarker(const FVector_NetQuantize& SpawnLocation);

	UFUNCTION(Server, reliable)
	void Server_DestroyMarker();

	UFUNCTION()
	void TODChanged(ETimeOfDay TOD);

	UFUNCTION()
	void KeyTimeHit(int32 CurrentTime);

	UFUNCTION(Client, reliable)
	void Client_RemoveRavenMarkerWidget();

public:

	UFUNCTION(Server, reliable, BlueprintCallable)
	virtual void Server_UnpossessMinion();

	UFUNCTION(BlueprintCallable)
	FORCEINLINE AWOGSpline* GetAssignedSplinePath() const { return AssignedSplinePath; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE TArray<AWOGRavenMarker*> GetSpawnedMarkersArray() { return SpawnedMarkers; }

	UPROPERTY(Replicated, BlueprintReadWrite, EditAnywhere)
	TArray<AWOGRavenMarker*> SpawnedMarkers;

};
