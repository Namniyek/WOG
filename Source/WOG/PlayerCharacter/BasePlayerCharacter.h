// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "WOG/Data/PlayerProfileSaveGame.h"
#include "InputActionValue.h"
#include "Engine/DataTable.h"
#include "WOG/Types/CharacterTypes.h"
#include "BasePlayerCharacter.generated.h"

USTRUCT(BlueprintType)
struct FMeshDataTables 
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	class UDataTable* MaleBody;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	UDataTable* FemaleBody;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	UDataTable* PrimaryColors;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	UDataTable* BodyPaintColor;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	UDataTable* SkinColor;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	UDataTable* HairColor;
};

USTRUCT(BlueprintType)
struct FMaterialColors : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FLinearColor ColorVector;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FLinearColor ColorVectorSec;
};

USTRUCT(BlueprintType)
struct FCharacterMesh : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	USkeletalMesh* BaseMesh;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	USkeletalMesh* Head;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	USkeletalMesh* ArmUpperLeft;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	USkeletalMesh* ArmUpperRight;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	USkeletalMesh* ArmLowerLeft;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	USkeletalMesh* ArmLowerRight;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	USkeletalMesh* HandLeft;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	USkeletalMesh* HandRight;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	USkeletalMesh* Torso;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	USkeletalMesh* Hips;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	USkeletalMesh* LegLeft;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	USkeletalMesh* LegRight;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	USkeletalMesh* Beard;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	USkeletalMesh* Eyebrows;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	USkeletalMesh* Hair;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	USkeletalMesh* Helmet;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	USkeletalMesh* Ears;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TSubclassOf<UAnimInstance> AnimBP;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TSubclassOf<UAnimInstance> AnimBPLobby;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	UAnimMontage* RaiseHandMontage;
};


UCLASS()
class WOG_API ABasePlayerCharacter : public ACharacter
{
	GENERATED_BODY()

	virtual void OnConstruction(const FTransform& Transform) override;

public:
	// Sets default values for this character's properties
	ABasePlayerCharacter();
	friend class UWOGAttributesComponent;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void Tick(float DeltaTime) override;

	UPROPERTY(ReplicatedUsing = OnRep_PlayerProfile, EditDefaultsOnly, BlueprintReadWrite)
	FPlayerData PlayerProfile;

#pragma region Handle Damage

	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	void Elim(bool bPlayerLeftGame);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Elim(bool bPlayerLeftGame);


#pragma endregion

	
	#pragma region Material variables
	UPROPERTY(EditAnywhere)
	UMaterialInterface* Material;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UMaterialInstanceDynamic* CharacterMI;

	#pragma endregion

	#pragma region Player Input Variables
	//Input
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	/** Dodge Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* DodgeAction;

	/** Sprint Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SprintAction;

	/** Target Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* TargetAction;

	#pragma endregion

protected:

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	#pragma region Components variables
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	USkeletalMeshComponent* MainMesh;

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

	#pragma endregion

	#pragma region Player Input Functions

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	/**Called for dodge input*/
	void Dodge(const FInputActionValue& Value);
	UFUNCTION(BlueprintCallable)
	void StopDodging();

	/**Called for sprint input*/
	void Sprint();
	void StopSprinting();

	/**Called for dodge input*/
	void Target(const FInputActionValue& Value);

	#pragma endregion

	#pragma region Player Profile Section
	UFUNCTION()
	void OnRep_PlayerProfile();

	UFUNCTION(BlueprintNativeEvent)
	void UpdatePlayerProfile(const FPlayerData& NewPlayerProfile);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Mesh")
	FMeshDataTables CharacterDataTables;

	void SetColors(FName Primary, FName Skin, FName BodyPaint, FName HairColor);
	void SetMeshes(bool bIsMale, FName RowName);


	#pragma endregion

	#pragma region Character State variables

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
	ECharacterState CharacterState;

	void SetCharacterState(ECharacterState NewState);

	virtual void HandleStateUnnoccupied();
	virtual void HandleStateDodging();
	virtual void HandleStateSprinting();
	virtual void HandleStateTargeting();

	#pragma endregion

	#pragma region Actor Components

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class ULockOnTargetComponent* LockOnTarget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class UTargetingHelperComponent* TargetAttractor;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class UWOGAttributesComponent* Attributes;

	#pragma endregion

	UFUNCTION()
	void TargetLocked(UTargetingHelperComponent* Target, FName Socket);

	UFUNCTION()
	void TargetUnlocked(UTargetingHelperComponent* UnlockedTarget, FName Socket);

	UFUNCTION()
	void TargetNotFound();

private:

	UPROPERTY()
	class AWOGGameMode* WOGGameMode;


public:
	//public Getters and Setters 
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	UFUNCTION(BlueprintPure)
	FORCEINLINE ECharacterState GetCharacterState() const { return CharacterState; }

	UFUNCTION(Server, reliable, BlueprintCallable)
	void Server_SetPlayerProfile(const FPlayerData& NewPlayerProfile);

	UFUNCTION(Server, reliable)
	void Server_SetCharacterState(ECharacterState NewState);

	UFUNCTION(NetMulticast, reliable)
	void Multicast_SetCharacterState(ECharacterState NewState);

};
