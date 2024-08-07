// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WOG/Data/PlayerProfileSaveGame.h"
#include "WOGLobbyAvatar.generated.h"

UCLASS()
class WOG_API AWOGLobbyAvatar : public AActor
{
	GENERATED_BODY()

	virtual void OnConstruction(const FTransform& Transform) override;
	
public:	
	AWOGLobbyAvatar();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(ReplicatedUsing = OnRep_AvatarTransform)
	FTransform AvatarTransform;

	UPROPERTY(ReplicatedUsing = OnRep_MeshProperties, BlueprintReadOnly)
	FPlayerData MeshProperties;

	UPROPERTY(EditAnywhere)
	UMaterialInterface* Material;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UMaterialInstanceDynamic* DynamicAvatarMaterial;

	UPROPERTY(BlueprintReadOnly)
	FText PlayerName;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	UAnimMontage* LobbyMontage;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	UStaticMeshComponent* RuneMesh;

protected:

	virtual void BeginPlay() override;

	UPROPERTY()
	USceneComponent* Root;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	class UTextRenderComponent* PlayerNameText;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	USkeletalMeshComponent* BaseMesh;

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

	UPROPERTY(EditDefaultsOnly)
	class URotatingMovementComponent* RotatingComponent;


private:
	UPROPERTY()
	class AWOGLobbyPlayerController* OwnerPC;

	UFUNCTION()
	void OnRep_MeshProperties();

	UFUNCTION()
	void OnRep_AvatarTransform();

	UFUNCTION(BlueprintCallable)
	void SetPlayerName(FText NewPlayerName);

	void SetTransform(FTransform NewTransform);

	void OnStartDelayFinished();

	bool ChangeTeams(AWOGLobbyPlayerController* OwnerPlayerController);


public:	

	UFUNCTION(BlueprintImplementableEvent)
	void UpdateCharacterMesh(FPlayerData NewMeshProperties);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_SetMeshProperties(FPlayerData NewMeshProperties);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_ChangeTeams();

	UFUNCTION(BlueprintCallable)
	void RotateLeftStart();
	UFUNCTION(BlueprintCallable)
	void RotateLeftStop();
	UFUNCTION(BlueprintCallable)
	void RotateRightStart();
	UFUNCTION(BlueprintCallable)
	void RotateRightStop();

	UFUNCTION(BlueprintCallable)
	void RaiseArm();

	UFUNCTION(BlueprintCallable)
	void LowerArm();

};
