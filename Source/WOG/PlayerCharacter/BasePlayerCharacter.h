// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WOG/Characters/WOGBaseCharacter.h"
#include "WOG/Data/PlayerProfileSaveGame.h"
#include "InputActionValue.h"
#include "Engine/DataTable.h"
#include "WOG/FunctionLibrary/MeshMergeFunctionLibrary.h"
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
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mesh")
	USkeletalMesh* BaseMesh;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mesh")
	USkeletalMesh* Head;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mesh")
	USkeletalMesh* ArmUpperLeft;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mesh")
	USkeletalMesh* ArmUpperRight;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mesh")
	USkeletalMesh* ArmLowerLeft;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mesh")
	USkeletalMesh* ArmLowerRight;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mesh")
	USkeletalMesh* HandLeft;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mesh")
	USkeletalMesh* HandRight;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mesh")
	USkeletalMesh* Torso;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mesh")
	USkeletalMesh* Hips;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mesh")
	USkeletalMesh* LegLeft;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mesh")
	USkeletalMesh* LegRight;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mesh")
	USkeletalMesh* Beard;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mesh")
	USkeletalMesh* Eyebrows;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mesh")
	USkeletalMesh* Hair;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mesh")
	USkeletalMesh* Helmet;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mesh")
	USkeletalMesh* Ears;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animations")
	TSubclassOf<UAnimInstance> AnimBP;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animations")
	TSubclassOf<UAnimInstance> AnimBPLobby;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animations")
	UAnimMontage* RaiseHandMontage;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animations")
	UAnimMontage* UnarmedHurtMontage;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animations")
	UAnimMontage* DodgeMontage;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animations")
	USkeleton* Skeleton;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animations")
	UPhysicsAsset* PhysicsAsset;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Initial Setup")
	TSubclassOf<class AWOGBaseWeapon> DefaultWeapon;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Initial Setup")
	FCharacterAbilityData DefaultAbilitiesAndEffects;
};


UCLASS()
class WOG_API ABasePlayerCharacter : public AWOGBaseCharacter
{
	GENERATED_BODY()

	virtual void OnConstruction(const FTransform& Transform) override;

public:
	// Sets default values for this character's properties
	ABasePlayerCharacter();
	friend class UWOGCombatComponent;
	friend class UWOGAbilitiesComponent;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void Tick(float DeltaTime) override;

	UPROPERTY(ReplicatedUsing = OnRep_PlayerProfile, EditDefaultsOnly, BlueprintReadWrite)
	FPlayerData PlayerProfile;

	#pragma region Handle Damage

	virtual void Elim(bool bPlayerLeftGame) override;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Elim(bool bPlayerLeftGame);

	UFUNCTION()
	void ElimTimerFinished();

	#pragma endregion
	
	#pragma region Material variables
	UPROPERTY(EditAnywhere)
	UMaterialInterface* Material;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UMaterialInstanceDynamic* CharacterMI;

	#pragma endregion

	#pragma region Player Input Variables
	/*
	** MappingContexts
	**/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Mapping Contexts", meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* MatchMappingContext;

	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Mapping Contexts", meta = (AllowPrivateAccess = "true"))
	//UInputMappingContext* RadialMenuMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Mapping Contexts", meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* SpawnModeMappingContext;

	/*
	** Input actions
	**/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Base Match", meta = (AllowPrivateAccess = "true"))
	class UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Base Match", meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Base Match", meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Base Match", meta = (AllowPrivateAccess = "true"))
	UInputAction* DodgeAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Base Match", meta = (AllowPrivateAccess = "true"))
	UInputAction* SprintAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Base Match", meta = (AllowPrivateAccess = "true"))
	UInputAction* TargetAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Base Match", meta = (AllowPrivateAccess = "true"))
	UInputAction* CycleTargetAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Base Match", meta = (AllowPrivateAccess = "true"))
	UInputAction* AbilitiesAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Base Match", meta = (AllowPrivateAccess = "true"))
	UInputAction* PrimaryLightAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Base Match", meta = (AllowPrivateAccess = "true"))
	UInputAction* PrimaryHeavyAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Base Match", meta = (AllowPrivateAccess = "true"))
	UInputAction* SecondaryAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Base Match", meta = (AllowPrivateAccess = "true"))
	UInputAction* RadialMenuAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Spawn Mode", meta = (AllowPrivateAccess = "true"))
	UInputAction* SpawnAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Spawn Mode", meta = (AllowPrivateAccess = "true"))
	UInputAction* RotateSpawnAction;

	#pragma endregion

protected:

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	#pragma region Components variables
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	#pragma endregion

	#pragma region Player Input Functions

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** Called for movement input */
	void MoveActionPressed(const FInputActionValue& Value);

	/** Called for looking input */
	void LookActionPressed(const FInputActionValue& Value);

	/** Called for jumping input */
	void JumpActionPressed(const FInputActionValue& Value);

	/**Called for dodge input*/
	void DodgeActionPressed(const FInputActionValue& Value);
	UFUNCTION(BlueprintCallable)
	void StopDodging();

	void Dodge();

	/**Called for sprint input*/
	void SprintActionPressed();
	void StopSprinting();

	/**Called for Target input*/
	void TargetActionPressed(const FInputActionValue& Value);
	void CycleTargetActionPressed(const FInputActionValue& Value);

	/**Called for equip input*/
	virtual void AbilitiesButtonPressed(const FInputActionValue& Value);

	UFUNCTION()
	void PrimaryLightButtonPressed(const FInputActionValue& Value);

	UFUNCTION()
	void PrimaryArmHeavyAttack(FInputActionValue ActionValue, float ElapsedTime, float TriggeredTime);
	void PrimaryHeavyAttackCanceled(const FInputActionValue& Value);

	void PrimaryExecuteHeavyAttack(const FInputActionValue& Value);
	void SecondaryButtonPressed(const FInputActionValue& Value);
	void SecondaryButtonReleased(const FInputActionValue& Value);
	
	#pragma endregion

	#pragma region Player Profile Section
	UFUNCTION()
	void OnRep_PlayerProfile();

	UFUNCTION(BlueprintNativeEvent)
	void UpdatePlayerProfile(const FPlayerData& NewPlayerProfile);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Mesh")
	FMeshDataTables CharacterDataTables;

	UPROPERTY()
	FSkeletalMeshMergeParams MergeParams;

	void SetColors(FName Primary, FName Skin, FName BodyPaint, FName HairColor);
	void SetMeshesAndAnimations(bool bIsMale, FName RowName);
	void SetDefaultAbilitiesAndEffects(bool bIsMale, FName RowName);


	#pragma endregion

	#pragma region Character State variables

	virtual void HandleStateUnnoccupied() override;
	virtual void HandleStateDodging() override;
	virtual void HandleStateSprinting() override;
	virtual void HandleStateElimmed(AController* InstigatedBy = nullptr) override;
	virtual void HandleStateAttacking() override;
	virtual void HandleStateStaggered() override;

	#pragma endregion

	#pragma region Actor Components

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class ULockOnTargetComponent* LockOnTarget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class UTargetingHelperComponent* TargetAttractor;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UWOGCombatComponent> Combat;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UWOGAbilitiesComponent> Abilities;


	#pragma endregion

	#pragma region Interface functions
	virtual void BroadcastHit_Implementation(AActor* AgressorActor, const FHitResult& Hit, const float& DamageToApply, AActor* InstigatorWeapon) override;

	#pragma endregion

	#pragma region Cosmetic Hits
	//Handle cosmetic body hit
	virtual void HandleCosmeticBodyHit(const FHitResult& Hit, const FVector& WeaponLocation, const class AWOGBaseWeapon* InstigatorWeapon) override;
	virtual void PlayHitReactMontage(FName Section) override;

	//Handle cosmetic block
	virtual void HandleCosmeticBlock(const AWOGBaseWeapon* InstigatorWeapon) override;

	//Handle cosmetic weapon clash
	virtual void HandleCosmeticWeaponClash() override;
	#pragma endregion

	UFUNCTION()
	void TargetLocked(UTargetingHelperComponent* Target, FName Socket);

	UFUNCTION()
	void TargetUnlocked(UTargetingHelperComponent* UnlockedTarget, FName Socket);

	UFUNCTION()
	void TargetNotFound();

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class AWOGPlayerController> OwnerPC = nullptr;
	TObjectPtr<AActor> CurrentTarget = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<class UAnimMontage> DodgeMontage;

private:




public:
	//public Getters and Setters 
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE UWOGAttributesComponent* GetAttributes() const { return Attributes; }
	FORCEINLINE ULockOnTargetComponent* GetLockOnTarget() const { return LockOnTarget; }
	FORCEINLINE UWOGCombatComponent* GetCombatComponent() const { return Combat; }
	FORCEINLINE bool GetIsTargeting() const { return bIsTargeting; }
	FORCEINLINE void SetOwnerPC(AWOGPlayerController* NewPC) { OwnerPC = NewPC; }
	FORCEINLINE TObjectPtr<AWOGPlayerController> GetOwnerPC() { return OwnerPC; }
	FORCEINLINE TObjectPtr<AActor>GetCurrentTarget() { return CurrentTarget; }

	UFUNCTION(Server, reliable, BlueprintCallable)
	void Server_SetPlayerProfile(const FPlayerData& NewPlayerProfile);



};
