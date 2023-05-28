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
	class UDataTable* MaleBody = nullptr;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	UDataTable* FemaleBody = nullptr;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	UDataTable* PrimaryColors = nullptr;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	UDataTable* BodyPaintColor = nullptr;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	UDataTable* SkinColor = nullptr;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	UDataTable* HairColor = nullptr;
};

USTRUCT(BlueprintType)
struct FMaterialColors : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FLinearColor ColorVector = FLinearColor::Black;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FLinearColor ColorVectorSec = FLinearColor::Black;
};

USTRUCT(BlueprintType)
struct FCharacterMesh : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mesh")
	USkeletalMesh* BaseMesh = nullptr;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mesh")
	USkeletalMesh* Head = nullptr;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mesh")
	USkeletalMesh* ArmUpperLeft = nullptr;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mesh")
	USkeletalMesh* ArmUpperRight = nullptr;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mesh")
	USkeletalMesh* ArmLowerLeft = nullptr;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mesh")
	USkeletalMesh* ArmLowerRight = nullptr;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mesh")
	USkeletalMesh* HandLeft = nullptr;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mesh")
	USkeletalMesh* HandRight = nullptr;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mesh")
	USkeletalMesh* Torso = nullptr;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mesh")
	USkeletalMesh* Hips = nullptr;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mesh")
	USkeletalMesh* LegLeft = nullptr;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mesh")
	USkeletalMesh* LegRight = nullptr;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mesh")
	USkeletalMesh* Beard = nullptr;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mesh")
	USkeletalMesh* Eyebrows = nullptr;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mesh")
	USkeletalMesh* Hair = nullptr;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mesh")
	USkeletalMesh* Helmet = nullptr;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mesh")
	USkeletalMesh* Ears = nullptr;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animations")
	TSubclassOf<UAnimInstance> AnimBP = nullptr;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animations")
	TSubclassOf<UAnimInstance> AnimBPLobby = nullptr;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animations")
	UAnimMontage* RaiseHandMontage = nullptr;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animations")
	UAnimMontage* UnarmedHurtMontage = nullptr;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animations")
	UAnimMontage* DodgeMontage = nullptr;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animations")
	USkeleton* Skeleton = nullptr;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animations")
	UPhysicsAsset* PhysicsAsset = nullptr;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Initial Setup")
	TSubclassOf<class AWOGBaseWeapon> DefaultWeapon = nullptr;
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

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PossessedBy(AController* NewController) override;

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
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup|Input|Mapping Contexts", meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* MatchMappingContext;

	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Mapping Contexts", meta = (AllowPrivateAccess = "true"))
	//UInputMappingContext* RadialMenuMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup|Input|Mapping Contexts", meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* SpawnModeMappingContext;

	/*
	** Input actions
	**/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup|Input|Base Match", meta = (AllowPrivateAccess = "true"))
	class UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup|Input|Base Match", meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup|Input|Base Match", meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup|Input|Base Match", meta = (AllowPrivateAccess = "true"))
	UInputAction* DodgeAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup|Input|Base Match", meta = (AllowPrivateAccess = "true"))
	UInputAction* SprintAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup|Input|Base Match", meta = (AllowPrivateAccess = "true"))
	UInputAction* TargetAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup|Input|Base Match", meta = (AllowPrivateAccess = "true"))
	UInputAction* CycleTargetAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup|Input|Base Match", meta = (AllowPrivateAccess = "true"))
	UInputAction* AbilitiesAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup|Input|Base Match", meta = (AllowPrivateAccess = "true"))
	UInputAction* PrimaryLightAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup|Input|Base Match", meta = (AllowPrivateAccess = "true"))
	UInputAction* PrimaryHeavyAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup|Input|Base Match", meta = (AllowPrivateAccess = "true"))
	UInputAction* SecondaryAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup|Input|Base Match", meta = (AllowPrivateAccess = "true"))
	UInputAction* RadialMenuAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Spawn Mode", meta = (AllowPrivateAccess = "true"))
	UInputAction* SpawnAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup|Input|Spawn Mode", meta = (AllowPrivateAccess = "true"))
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

	/**Called for sprint input*/
	void SprintActionPressed();
	void StopSprinting();

	/**Called for Target input*/
	void TargetActionPressed(const FInputActionValue& Value);
	void CycleTargetActionPressed(const FInputActionValue& Value);

	/**Called for equip input*/
	virtual void AbilitiesButtonPressed(const FInputActionValue& Value) { /*TO-BE OVERRIDEN IN CHILDREN*/ };

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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup|Character Data")
	FMeshDataTables CharacterDataTables;

	UPROPERTY()
	FSkeletalMeshMergeParams MergeParams;

	void SetColors(FName Primary, FName Skin, FName BodyPaint, FName HairColor);
	void SetMeshesAndAnimations(bool bIsMale, FName RowName);
	void SetDefaultAbilitiesAndEffects(bool bIsMale, FName RowName);

	#pragma endregion

	#pragma region Inventory
protected:

	UFUNCTION(Client, reliable, BlueprintCallable)
	void Client_SaveShortcutReferences(AActor* InItem, const FGameplayTag& InItemTag, const FName& Key);

public:
	UFUNCTION(Server, reliable, BlueprintCallable)
	void Server_EquipWeapon(const FName& Key, AActor* InWeapon);

	UFUNCTION(Server, reliable, BlueprintCallable)
	void Server_UnequipWeapon(const FName& Key, AActor* InWeapon);

	#pragma endregion

	#pragma region Character State variables

protected:
	virtual void HandleStateElimmed(AController* InstigatedBy = nullptr) override;
	virtual void HandleStateStaggered() override;

	#pragma endregion

	#pragma region Actor Components

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class ULockOnTargetComponent* LockOnTarget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class UTargetingHelperComponent* TargetAttractor;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<class UAGR_EquipmentManager> EquipmentManager;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<class UAGR_InventoryManager> InventoryManager;

	#pragma endregion

	#pragma region Interface functions
	virtual void BroadcastHit_Implementation(AActor* AgressorActor, const FHitResult& Hit, const float& DamageToApply, AActor* InstigatorWeapon) override;

	#pragma endregion

	virtual void ProcessHit(FHitResult Hit, UPrimitiveComponent* WeaponMesh) override;

	#pragma region Cosmetic Hits
	//Handle cosmetic body hit
	virtual void HandleCosmeticBodyHit(const FHitResult& Hit, const FVector& WeaponLocation, const class AWOGBaseWeapon* InstigatorWeapon) override;
	virtual void PlayHitReactMontage(FName Section) override;

	//Handle cosmetic block
	virtual void HandleCosmeticBlock(const AWOGBaseWeapon* InstigatorWeapon) override;
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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup|Animations")
	TObjectPtr<class UAnimMontage> DodgeMontage;

public:
	//public Getters and Setters 
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE ULockOnTargetComponent* GetLockOnTarget() const { return LockOnTarget; }
	FORCEINLINE bool GetIsTargeting() const { return bIsTargeting; }
	FORCEINLINE void SetOwnerPC(AWOGPlayerController* NewPC) { OwnerPC = NewPC; }
	FORCEINLINE TObjectPtr<AWOGPlayerController> GetOwnerPC() { return OwnerPC; }
	FORCEINLINE TObjectPtr<AActor>GetCurrentTarget() { return CurrentTarget; }

	UFUNCTION(Server, reliable, BlueprintCallable)
	void Server_SetPlayerProfile(const FPlayerData& NewPlayerProfile);



};
