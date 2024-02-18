// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WOGBaseActorComponent.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "Data/WOGDataTypes.h"
#include "WOGBuildComponent.generated.h"

/**
 * 
 */

 /** Struct used to define the types of buildables available */
USTRUCT(BlueprintType)
struct FBuildables : public FTableRowBase
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Preview Mesh", MakeStructureDefaultValue = "None"), Category = "1 - Base")
	TObjectPtr<UStaticMesh> Mesh = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Extension Mesh", MakeStructureDefaultValue = "None"), Category = "1 - Base")
	TObjectPtr<UStaticMesh> ExtensionMesh = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "TraceChannel", MakeStructureDefaultValue = "TraceTypeQuery1"), Category = "1 - Base")
	TEnumAsByte<ETraceTypeQuery> TraceChannel = ETraceTypeQuery::TraceTypeQuery1;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Build Actor", MakeStructureDefaultValue = "None"), Category = "1 - Base")
	TObjectPtr<UClass> Actor = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Health", MakeStructureDefaultValue = "0.000000"), Category = "1 - Base")
	float Health = 0.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "MaxHeightOffset", MakeStructureDefaultValue = "0.000000"), Category = "1 - Base")
	float MaxHeightOffset = 0.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "AvoidAddingAsChild", MakeStructureDefaultValue = "False"), Category = "1 - Base")
	bool AvoidAddingAsChild = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Build Item Tag"), Category = "2 - Inventory")
	FGameplayTag ItemTag = FGameplayTag();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Build Name"), Category = "3 - User Interface")
	FText Name = FText();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Build Icon"), Category = "3 - User Interface")
	TObjectPtr<UTexture2D> Icon = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Build cost amount"), Category = "1 - Base")
	int32 CostAmount = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Build cost tag"), Category = "1 - Base")
	FGameplayTag CostTag = FGameplayTag();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3 - User Interface")
	FVendorItemData VendorItemData = FVendorItemData();
};

class UCameraComponent;


UCLASS()
class WOG_API UWOGBuildComponent : public UWOGBaseActorComponent
{
	GENERATED_BODY()

public:
	friend class AWOGDefender;
	UWOGBuildComponent();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:

	FTimerHandle BuildTimerHandle;
	TObjectPtr<AWOGDefender> DefenderCharacter;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Setup")
	TObjectPtr<UMaterialInstance> AllowedGhostMaterial;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Setup")
	TObjectPtr<UMaterialInstance> ForbiddenGhostMaterial;

	UPROPERTY(Replicated)
	TArray<FBuildables> Buildables;

public:

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default")
	bool bIsBuildModeOn;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default")
	bool bCanBuild;

	UPROPERTY(Replicated, BlueprintReadWrite, EditDefaultsOnly, Category = "Default")
	int32 BuildID;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default")
	FTransform BuildTransform;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default")
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default")
	TObjectPtr<UStaticMeshComponent> BuildGhost;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default")
	TObjectPtr<AActor> CurrentHitActor;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default")
	TObjectPtr<UPrimitiveComponent> CurrentHitComponent;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Default")
	FVector HeightOffset;

protected:
	virtual void BeginPlay() override;

	void StopBuildMode();
	void BuildDelay();
	void BuildCycle();

	void SpawnBuildGhost();
	void GiveBuildColor(bool IsAllowed);

	void DetectBuildBoxes(bool& OutFound, FTransform& OutTransform);
	bool CheckForOverlap();
	bool IsBuildFloating();
	bool CheckCost();

	void DeductCost();

	UFUNCTION(BlueprintCallable)
	void ChangeMesh(int32 ID);

	UFUNCTION(BlueprintCallable, Server, reliable)
	void Server_SpawnBuild(FTransform Transform, int32 ID, AActor* Hit, UPrimitiveComponent* HitComponent);
	void SpawnBuild(FTransform Transform, int32 ID, AActor* Hit, UPrimitiveComponent* HitComponent);

	UFUNCTION(Server, reliable)
	void Server_SetBuildables(const TArray<FBuildables>& InBuildables);
	bool SetBuildables();

public:

	UFUNCTION(BlueprintCallable)
	void LaunchBuildMode();

	UFUNCTION(BlueprintCallable)
	void PlaceBuildable();

	UFUNCTION(BlueprintCallable)
	void HandleBuildHeight(bool bShouldRise);

	UFUNCTION(BlueprintCallable)
	void HandleBuildRotation(bool bRotateLeft);

	UFUNCTION(BlueprintCallable, Server, reliable)
	void Server_InteractWithBuild(UObject* HitActor);

	UFUNCTION(BlueprintPure)
	FORCEINLINE TArray<FBuildables> GetBuildables() const { return Buildables; }

};
