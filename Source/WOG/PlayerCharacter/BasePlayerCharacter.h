// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WOG/Characters/WOGBaseCharacter.h"
#include "WOG/Data/PlayerProfileSaveGame.h"
#include "InputActionValue.h"
#include "Engine/DataTable.h"
#include "WOG/FunctionLibrary/MeshMergeFunctionLibrary.h"
#include "Types/CharacterTypes.h"
#include "Interfaces/InventoryInterface.h"
#include "BasePlayerCharacter.generated.h"


class AWOGPingMarker;
class AWOGCommonInventory;
class AWOGVendor;
class AWOGStashBase;
class AWOGPlayerController;
class UTargetSystemComponent;
class AWOGBaseSquad;

USTRUCT(BlueprintType)
struct FMeshDataTables 
{
	GENERATED_BODY()

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

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FLinearColor ColorVector = FLinearColor::Black;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FLinearColor ColorVectorSec = FLinearColor::Black;
};

USTRUCT(BlueprintType)
struct FCharacterMesh : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

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
	USkeleton* Skeleton = nullptr;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animations")
	UPhysicsAsset* PhysicsAsset = nullptr;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Initial Setup")
	TSubclassOf<class AWOGBaseWeapon> DefaultWeapon = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Initial Setup")
	FCharacterAbilityData DefaultAbilitiesAndEffects;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Initial Setup")
	FCharacterData 	CharacterData;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractComplete, ABasePlayerCharacter*, Interactor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerInitComplete, const ABasePlayerCharacter*, InitCharacter);

UCLASS()
class WOG_API ABasePlayerCharacter : public AWOGBaseCharacter, public IInventoryInterface
{
	GENERATED_BODY()

	friend class AWOGBaseCharacter;

	virtual void OnConstruction(const FTransform& Transform) override;

public:
	// Sets default values for this character's properties
	ABasePlayerCharacter();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PostInitializeComponents() override;

	#pragma region Handle Elim

	virtual void Elim(bool bPlayerLeftGame) override;
	
	virtual void HandleStateElimmed(AController* InstigatedBy = nullptr) override;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Elim(bool bPlayerLeftGame);

	UFUNCTION()
	void ElimTimerFinished();

#pragma endregion

	#pragma region Player Input Variables
	/*
	** MappingContexts
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup|Input|Mapping Contexts", meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* SpawnModeMappingContext;

	/*
	** Input actions
	*/
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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup|Input|Spawn Mode", meta = (AllowPrivateAccess = "true"))
	UInputAction* SpawnAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup|Input|Spawn Mode", meta = (AllowPrivateAccess = "true"))
	UInputAction* AlternativeAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup|Input|Spawn Mode", meta = (AllowPrivateAccess = "true"))
	UInputAction* AlternativeModeAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup|Input|Spawn Mode", meta = (AllowPrivateAccess = "true"))
	UInputAction* ChangeActiveSquadAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup|Input|Spawn Mode", meta = (AllowPrivateAccess = "true"))
	UInputAction* RotateSpawnAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup|Input|Base Match", meta = (AllowPrivateAccess = "true"))
	UInputAction* WeaponRangedAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup|Input|Base Match", meta = (AllowPrivateAccess = "true"))
	UInputAction* PingAction;

	/*
	** Other variables
	*/
	UPROPERTY(BlueprintReadOnly)
	bool bIsAlternativeModeEnabled = false;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnInteractComplete OnInteractComplete;
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

	UFUNCTION(BlueprintCallable)
	void MoveCharacter(const FVector2D& MovementVector);

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

	void WeaponRangedActionPressed(const FInputActionValue& Value);

	/**Called for equip input*/
	virtual void AbilitiesButtonPressed(const FInputActionValue& Value) { /*To be overriden in children*/ };
	
	/**Called for hold ability input*/
	virtual void Ability2HoldButtonStarted(const FInputActionValue& Value) { /*To be overriden in children*/ };
	virtual void Ability2HoldButtonTriggered(const FInputActionValue& Value) { /*To be overriden in children*/ };
	virtual void Ability3HoldButtonStarted(const FInputActionValue& Value) { /*To be overriden in children*/ };
	virtual void Ability3HoldButtonTriggered(const FInputActionValue& Value) { /*To be overriden in children*/ };

	void AbilityHoldStarted(const FName& Slot);

	UFUNCTION()
	void AbilityHoldButtonElapsed(FInputActionValue ActionValue, float ElapsedTime, float TriggeredTime);
	void AbilityHoldButtonCanceled(const FInputActionValue& Value);

	UFUNCTION()
	void PrimaryLightButtonPressed(const FInputActionValue& Value);
	void PrimaryHeavyAttackStarted(const FInputActionValue& Value);
	void ConfirmHoldStarted();

	UFUNCTION()
	void PrimaryArmHeavyAttack(FInputActionValue ActionValue, float ElapsedTime, float TriggeredTime);
	void PrimaryHeavyAttackCanceled(const FInputActionValue& Value);

	void PrimaryExecuteHeavyAttack(const FInputActionValue& Value);
	void SecondaryButtonPressed(const FInputActionValue& Value);
	void SecondaryButtonReleased(const FInputActionValue& Value);
	
	void AlternativeModeActionPressed(const FInputActionValue& Value);
	void AlternativeModeActionReleased(const FInputActionValue& Value);
	virtual void AlternativeModeEnabled(const bool& NewEnabled) { /*To be overriden in children*/ };

	virtual void ChangeActiveSquadActionPressed(const FInputActionValue& Value) { /*To be overriden in children*/ };
	virtual void AlternativeActionPressed(const FInputActionValue& Value) { /*To be overriden in children*/ };

	void PingActionPressed(const FInputActionValue& Value);
	#pragma endregion

	#pragma region Player Profile Section
public:
	UFUNCTION(Server, reliable, BlueprintCallable)
	void Server_SetPlayerProfile(const FPlayerData& NewPlayerProfile);

	UFUNCTION(BlueprintNativeEvent)
	void UpdatePlayerProfile(const FPlayerData& NewPlayerProfile);

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnPlayerInitComplete OnPlayerInitComplete;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_PlayerProfile, EditDefaultsOnly, BlueprintReadWrite)
	FPlayerData PlayerProfile;

	UFUNCTION()
	void OnRep_PlayerProfile();

	UFUNCTION(BlueprintNativeEvent)
	void HandleOnPlayerInitComplete(const ABasePlayerCharacter* InitCharacter);

	void HandleTeamUIElements() const;

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

	UPROPERTY(Replicated)
	FName PreviousWeapon;
	UPROPERTY(Replicated)
	FName PreviousMagic;
	UPROPERTY(Replicated)
	FName CurrentMagic;

	UFUNCTION(BlueprintCallable)
	void ResetPreviouslyEquippedMaterial();

	UFUNCTION(BlueprintCallable)
	void UnequipMagic(const bool& bIsAttacker, const FName& Slot);
	UFUNCTION(BlueprintCallable)
	void UnequipWeapon(const bool& bIsAttacker, const FName& Slot);
	UFUNCTION(BlueprintCallable)
	void EquipMagic(const FName& Slot);
	UFUNCTION(BlueprintCallable)
	void EquipWeapon(const FName& Slot);

	UFUNCTION()
	void OnConsumablePickedUp(AActor* Item);

	
	void HandleEquipWeapon(const FName& Key, AActor* InWeapon);
	void HandleUnequipWeapon(const FName& Key, AActor* InWeapon);
	void HandleEquipMagic(const FName& Key, AActor* InMagic);
	void HandleUnequipMagic(const FName& Key, AActor* InMagic) const;

public:
	UFUNCTION(Server, reliable, BlueprintCallable)
	void Server_EquipWeapon(const FName& Key, AActor* InWeapon);

	UFUNCTION(Server, reliable, BlueprintCallable)
	void Server_UnequipWeapon(const FName& Key, AActor* InWeapon);

	UFUNCTION(Server, reliable, BlueprintCallable)
	void Server_EquipMagic(const FName& Key, AActor* InMagic);

	UFUNCTION(Server, reliable, BlueprintCallable)
	void Server_UnequipMagic(const FName& Key, AActor* InMagic);

	UFUNCTION(Server, reliable, BlueprintCallable)
	void Server_UnequipWeaponSwap(const FName& Key, AActor* InWeapon);

	UFUNCTION(Server, reliable, BlueprintCallable)
	void Server_DropWeapon(const FName& Key);

	UFUNCTION(Server, reliable, BlueprintCallable)
	void Server_DropMagic(const FName& Key);

	UFUNCTION(Server, reliable, BlueprintCallable)
	void Server_StoreWeapons();

	UFUNCTION(Server, reliable, BlueprintCallable)
	void Server_RestoreWeapons();

	void StoreTool(const FName& Key);
	void RestoreTools();

	void StoreWeapon(const FName& Key);
	void RestoreWeapons();

	void StoreMagic(const FName& Key);
	void RestoreMagic ();

	void StoreConsumable();
	void RestoreConsumable();
	void EquipConsumable(AActor* InItem);
	void UnequipConsumable();

	void StoreEquipment();
	void RestoreEquipment();

	UFUNCTION()
	void CreateDefaults();

	UFUNCTION(BlueprintCallable)
	void CreateDefaultTools();

	UFUNCTION()
	void GrantDefaultMagics();

	void DestroyDefaultToolsAndMagics();

	UFUNCTION()
	void HandleWeaponSwitch(bool bStoreWeapons);

	void SendEquipmentToCommonInventory() const;
	void RestoreEquipmentFromCommonInventory();
	#pragma endregion

	#pragma region Actor Components
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UTargetSystemComponent* TargetComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<class UAGR_EquipmentManager> EquipmentManager;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<class UAGR_InventoryManager> InventoryManager;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAGR_CombatManager> CombatManager;

	#pragma endregion

	#pragma region Handle Combat
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup|Combat")
	TSubclassOf<UGameplayEffect> DamageEffectForSelf;
	
	UFUNCTION()
	virtual void OnStartAttack();
	TArray<TObjectPtr<AActor>> HitActorsToIgnore;

	UPROPERTY(Replicated, VisibleAnywhere)
	TObjectPtr<AWOGBaseSquad> CurrentEpicSquad;

	UFUNCTION()
	virtual void OnAttackHit(FHitResult Hit, UPrimitiveComponent* WeaponMesh);

	virtual void BroadcastHit_Implementation(AActor* AggressorActor, const FHitResult& Hit, const float& DamageToApply, AActor* InstigatorWeapon) override;
	void HandleHitFromPlayerCharacter(AActor* AggressorActor, const FHitResult& Hit, const float& DamageToApply, AActor* InstigatorWeapon);
	void HandleHitFromEnemyCharacter(AActor* AggressorActor, const FHitResult& Hit, const float& DamageToApply, AActor* InstigatorWeapon);
	virtual void BroadcastMagicHit_Implementation(AActor* AggressorActor, const FHitResult& Hit, const struct FMagicDataTable& AggressorMagicData) override;
	virtual void SetCurrentEpicSquadSlot_Implementation(AWOGBaseSquad* NewSquad) override;
	virtual void FreeCurrentEpicSquadSlot_Implementation() override;
	virtual bool IsCurrentEpicSquadSlotAvailable_Implementation() const override;
	virtual AWOGBaseSquad* GetCurrentEpicSquadSlot_Implementation() const override;
	
	virtual void ProcessHit(FHitResult Hit, UPrimitiveComponent* WeaponMesh) override;
	virtual void ProcessMagicHit(const FHitResult& Hit, const struct FMagicDataTable& MagicData) override;
	virtual void ProcessRangedHit(const FHitResult& Hit, const float& DamageToApply, AActor* AggressorWeapon) override;
	#pragma endregion

	#pragma region Handle Resources 

	UPROPERTY(VisibleAnywhere, Replicated)
	TObjectPtr<AWOGCommonInventory> CommonInventory;

	void FindCommonInventory();

	UFUNCTION(Server, reliable, BlueprintCallable)
	void Server_CollectResource(UAGR_ItemComponent* ItemToCollect);
	#pragma endregion

	#pragma region Vendors
	virtual void BuyItem_Implementation(const TArray<FCostMap>& CostMap, AWOGVendor* VendorActor, TSubclassOf<AActor> ItemClass, const int32& Amount, bool bIsUpgrade, const int32& NewLevel, const FGameplayTag& ItemTag) override;

	virtual void TransactionComplete_Implementation() override;

	UFUNCTION(Server, reliable)
	void Server_BuyItem(const TArray<FCostMap>& CostMap, AWOGVendor* VendorActor, TSubclassOf<AActor> ItemClass, const int32& Amount, bool bIsUpgrade, const int32& NewLevel, const FGameplayTag& ItemTag);

	UFUNCTION(Server, reliable)
	void Server_SetVendorBusy(bool bNewBusy, ABasePlayerCharacter* UserPlayer, AWOGVendor* Vendor);

	UFUNCTION(Client, reliable)
	void Client_KickPlayerFromVendor(AWOGVendor* Vendor);
	#pragma endregion

	#pragma region Stash
	UFUNCTION(Server, reliable)
	void Server_SetStashBusy(bool bNewBusy, ABasePlayerCharacter* UserPlayer, AWOGStashBase* Stash);

	UFUNCTION(Server, reliable)
	void Server_SwitchItem(AWOGStashBase* Stash, bool bToCommon, AActor* ItemToSwitch, AActor* PreviousItem, FGameplayTagContainer AuxTagsContainer, TSubclassOf<AActor> ItemClass, const int32& Amount);
	
	UFUNCTION(Client, reliable)
	void Client_KickPlayerFromStash(AWOGStashBase* Stash);
	#pragma endregion

	#pragma region TimeOfDay
	virtual void HandleTODChange() override;
	#pragma endregion

	#pragma region Animation
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup|Animations")
	float SpeedRequiredForLeap;
	#pragma endregion

	#pragma region UI
	UFUNCTION()
	void AddHoldProgressBar() const;
	UFUNCTION()
	void RemoveHoldProgressBarWidget() const;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UWidgetComponent> StaminaWidgetContainer;

	UFUNCTION(Server, Unreliable, BlueprintCallable)
	void Server_SpawnPingActor(const FVector_NetQuantize& SpawnLocation);

	UPROPERTY(EditDefaultsOnly, Category = "Setup | Ping")
	TSubclassOf<AWOGPingMarker> PingMarkerClass; 
	#pragma endregion

	#pragma region Teleport
	UFUNCTION()
	void FinishTeleportCharacter(const FTransform& Destination);
	
	UFUNCTION(BlueprintCallable, Server, reliable)
	void Server_StartTeleportCharacter(const FTransform& Destination);
	#pragma endregion

	#pragma region Targeting
	UFUNCTION()
	void TargetLocked(AActor* NewTarget);

	UFUNCTION()
	void TargetUnlocked(AActor* OldTarget);

	UFUNCTION(Server, reliable)
	void Server_SetCurrentTarget(AActor* NewTarget = nullptr);

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Replicated)
	TObjectPtr<AActor> CurrentTarget = nullptr;
	#pragma endregion

	#pragma region Interface functions
	virtual int32 GetAvailableAttackTokens_Implementation() override;
	virtual bool ReserveAttackTokens_Implementation(const int32& AmountToReserve) override;
	virtual void RestoreAttackTokens_Implementation(const int32& AmountToRestore) override;
	#pragma endregion

public:
	//public Getters and Setters 
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE UTargetSystemComponent* GetTargetComponent() const { return TargetComponent; }
	FORCEINLINE TObjectPtr<AActor>GetCurrentTarget() const { return CurrentTarget; }
	FORCEINLINE FPlayerData GetPlayerProfile() const { return PlayerProfile; }
	UFUNCTION(BlueprintPure)
	FORCEINLINE AWOGCommonInventory* GetCommonInventory() const { return CommonInventory; }
	FORCEINLINE float GetSpeedRequiredForLeap() const { return SpeedRequiredForLeap; }

	//Can return nullptr
	UWOGCharacterWidgetContainer* GetStaminaWidgetContainer() const;

	UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable)
	void SetCurrentEpicSquad(AWOGBaseSquad* NewSquad);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_TargetLockOff();

};
