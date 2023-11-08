// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "GameplayAbilitySpec.h"
#include "Types/CharacterTypes.h"
#include "WOGBaseConsumable.generated.h"

class UGameplayEffect;
class UWOGGameplayAbilityBase;
class UAnimMontage;
class USoundCue;
class UNiagaraSystem;
class ABasePlayerCharacter;
class UAGR_ItemComponent;
class UAGR_InventoryManager;
class USphereComponent;

USTRUCT(BlueprintType)
struct FConsumableDataTable : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1 - Base")
	bool bIsAttacker = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1 - Base")
	UStaticMesh* MeshMain = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "2 - Animations")
	UAnimMontage* UseMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "2 - Animations")
	FName UseMontageSection = FName("");

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "2 - Animations")
	float AnimationSpeed = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3 - GAS")
	FGameplayTag ConsumableTag = FGameplayTag();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3 - GAS")
	TSubclassOf<UGameplayEffect> ConsumeEffect = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3 - GAS")
	TSubclassOf<UGameplayEffect> SecondaryEffect = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "3 - GAS")
	float SecondaryEffectDuration = 0.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "3 - GAS")
	TArray<TSubclassOf<UWOGGameplayAbilityBase>> AbilitiesToGrant = { nullptr };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3 - GAS")
	FGameplayTag CooldownTag = FGameplayTag();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "4 - Stats")
	float Value = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "4 - Stats")
	float ValueMultiplier = 0.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "4 - Stats")
	float Cooldown = 0.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "4 - Stats")
	float Duration = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "5 - Cosmetic")
	USoundCue* ConsumeSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "5 - Cosmetic")
	TObjectPtr<UNiagaraSystem> ConsumeVFX = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "6 - User Interface")
	TSubclassOf<UUserWidget> AbilityWidgetClass = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "6 - User Interface")
	UTexture2D* AbilityIcon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "6 - User Interface")
	FVendorItemData VendorItemData = FVendorItemData();
};

UCLASS()
class WOG_API AWOGBaseConsumable : public AActor
{
	GENERATED_BODY()
	
public:	

	friend class ABasePlayerCharacter;
	AWOGBaseConsumable();
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void PostInitializeComponents();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;

	#pragma region ActorComponents

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAGR_ItemComponent> ItemComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<USphereComponent> SphereComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> Mesh;
	#pragma endregion

	#pragma region ConsumableVariables
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName ConsumableName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FConsumableDataTable ConsumableData;
	#pragma endregion

	#pragma region Item Functions
	UFUNCTION()
	void OnConsumableOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnConsumablePickedUp(UAGR_InventoryManager* Inventory);

	UFUNCTION()
	void OnConsumableUsed(AActor* User, FGameplayTag GameplayTag);

	UFUNCTION()
	void OnConsumableDestroyed();
	#pragma endregion

	#pragma region GAS
	UFUNCTION(BlueprintCallable)
	bool GrantAbilities();
	bool RemoveGrantedAbilities(AActor* User);

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TArray<FGameplayAbilitySpecHandle> GrantedAbilities;
	#pragma endregion

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<ABasePlayerCharacter> OwnerCharacter;

private:
	virtual void InitConsumableData();
	void UpdateVendorData(FConsumableDataTable* Row);

	void AddAbilityWidget(const int32& Key);
	UPROPERTY(Replicated)
	int32 AbilityKey = 1;

public:	
	UFUNCTION(BlueprintPure)
	FORCEINLINE FConsumableDataTable GetConsumableData() const { return ConsumableData; }

	UFUNCTION(BlueprintCallable)
	void SetOwnerCharacter(ABasePlayerCharacter* NewOwner);
	UFUNCTION(BlueprintPure)
	FORCEINLINE ABasePlayerCharacter* GetOwnerCharacter() const { return OwnerCharacter; }
};
