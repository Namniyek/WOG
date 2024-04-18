// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTags.h"
#include "GameplayAbilitySpec.h"
#include "WOGBaseInventoryItem.generated.h"

class UAGR_ItemComponent;
class ABasePlayerCharacter;

UCLASS()
class WOG_API AWOGBaseInventoryItem : public AActor
{
	GENERATED_BODY()

public:	

	AWOGBaseInventoryItem();
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void PostInitializeComponents() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:

	virtual void BeginPlay() override;
	virtual void InitData() {/*TO BE OVERRIDEN IN CHILDREN*/ };

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<ABasePlayerCharacter> OwnerCharacter;

	#pragma region Actor Components

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UAGR_ItemComponent* ItemComponent;
	#pragma endregion

	#pragma region Item Variables
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup")
	TArray<FName> ItemNames;
	
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
	int32 ItemLevel;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup")
	TObjectPtr<UDataTable> ItemDataTable;
	#pragma endregion

	#pragma region Item Functions
	UFUNCTION()
	virtual void OnItemPickedUp(UAGR_InventoryManager* Inventory) {/*TO BE OVERRIDEN IN CHILDREN*/ };

	UFUNCTION()
	virtual void OnItemEquipped(AActor* User, FName SlotName) {/*TO BE OVERRIDEN IN CHILDREN*/ };

	UFUNCTION()
	virtual void OnItemUnequipped(AActor* User, FName SlotName) {/*TO BE OVERRIDEN IN CHILDREN*/ };

	UFUNCTION()
	virtual void OnItemUsed(AActor* User, FGameplayTag GameplayTag) {/*TO BE OVERRIDEN IN CHILDREN*/ };

	UFUNCTION()
	virtual void OnItemDestroyed() {/*TO BE OVERRIDEN IN CHILDREN*/ };
	#pragma endregion

	#pragma region GAS
	UFUNCTION(BlueprintCallable)
	virtual bool GrantAbilities(AActor* User) { return false; /*TO BE OVERRIDEN IN CHILDREN*/ };
	virtual bool RemoveGrantedAbilities(AActor* User) { return false; /*TO BE OVERRIDEN IN CHILDREN*/ };

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TArray<FGameplayAbilitySpecHandle> GrantedAbilities;
	#pragma endregion

public:	

	UFUNCTION(BlueprintCallable)
	void SetOwnerCharacter(ABasePlayerCharacter* NewOwner);
	UFUNCTION(BlueprintPure)
	FORCEINLINE ABasePlayerCharacter* GetOwnerCharacter() const { return OwnerCharacter; }

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void SetItemLevel(FGameplayTag ItemTag, int32 NewLevel);

	UFUNCTION(BlueprintPure)
	FORCEINLINE int32 GetItemLevel() const { return ItemLevel; }
};
