// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WOGBaseActorComponent.h"
#include "Engine/DataTable.h"
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
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Mesh", MakeStructureDefaultValue = "None"))
	TObjectPtr<UStaticMesh> Mesh = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "ExtensionMesh", MakeStructureDefaultValue = "None"))
	TObjectPtr<UStaticMesh> ExtensionMesh = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "TraceChannel", MakeStructureDefaultValue = "TraceTypeQuery1"))
	TEnumAsByte<ETraceTypeQuery> TraceChannel = ETraceTypeQuery::TraceTypeQuery1;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Actor", MakeStructureDefaultValue = "None"))
	TObjectPtr<UClass> Actor = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Health", MakeStructureDefaultValue = "0.000000"))
	double Health = 0.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "MaxHeightOffset", MakeStructureDefaultValue = "0.000000"))
	double MaxHeightOffset = 0.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "AvoidAddingAsChild", MakeStructureDefaultValue = "False"))
	bool AvoidAddingAsChild = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Name"))
	FText Name = FText();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Icon"))
	TObjectPtr<UTexture2D> Icon = nullptr;

};

class UCameraComponent;


UCLASS()
class WOG_API UWOGBuildComponent : public UWOGBaseActorComponent
{
	GENERATED_BODY()

public:
	friend class AWOGDefender;
	UWOGBuildComponent();

private:

	FTimerHandle BuildTimerHandle;
	TObjectPtr<AWOGDefender> DefenderCharacter;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Building Setting")
	TObjectPtr<UDataTable> BuildablesDataTable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Building Setting")
	TObjectPtr<UMaterialInstance> AllowedGhostMaterial;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Building Setting")
	TObjectPtr<UMaterialInstance> ForbiddenGhostMaterial;

	TArray<FBuildables*> Buildables;

public:

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	int32 LastIndexDataTable = 0;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default")
	bool bIsBuildModeOn;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default")
	bool bCanBuild;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Default")
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

	UFUNCTION(BlueprintCallable)
	void ChangeMesh();

	UFUNCTION(Server, reliable)
	void Server_SpawnBuild(FTransform Transform, int32 ID, AActor* Hit, UPrimitiveComponent* HitComponent);
	void SpawnBuild(FTransform Transform, int32 ID, AActor* Hit, UPrimitiveComponent* HitComponent);

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

};
