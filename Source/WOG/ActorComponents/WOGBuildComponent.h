// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WOGBaseActorComponent.h"
#include "Engine/DataTable.h"
#include "WOGBuildComponent.generated.h"

/**
 * 
 */
 /** Please add a struct description */
USTRUCT(BlueprintType)
struct FBuildables : public FTableRowBase
{
	GENERATED_BODY()
public:
	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Mesh", MakeStructureDefaultValue = "None"))
	TObjectPtr<UStaticMesh> Mesh;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "ExtensionMesh", MakeStructureDefaultValue = "None"))
	TObjectPtr<UStaticMesh> ExtensionMesh;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "TraceChannel", MakeStructureDefaultValue = "TraceTypeQuery1"))
	TEnumAsByte<ETraceTypeQuery> TraceChannel;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Actor", MakeStructureDefaultValue = "None"))
	TObjectPtr<UClass> Actor;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Health", MakeStructureDefaultValue = "0.000000"))
	double Health;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "MaxHeightOffset", MakeStructureDefaultValue = "0.000000"))
	double MaxHeightOffset;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "AvoidAddingAsChild", MakeStructureDefaultValue = "False"))
	bool AvoidAddingAsChild;
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


public:

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	int32 LastIndexDataTable = 0;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Default")
	bool bIsBuildModeOn;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Default")
	bool bCanBuild;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Default")
	int32 BuildID;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Default")
	FTransform BuildTransform;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Default")
	TObjectPtr<UCameraComponent> Camera;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default")
	TObjectPtr<UStaticMeshComponent> BuildGhost;

	/** Please add a variable description */
	TArray<FBuildables*> Buildables;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Default")
	TObjectPtr<AActor> CurrentHitActor;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Default")
	TObjectPtr<UPrimitiveComponent> CurrentHitComponent;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Default")
	FVector HeightOffset;

protected:
	virtual void BeginPlay() override;

	/** Please add a function description */
	UFUNCTION(BlueprintCallable)
	void SpawnBuildGhost();

	/** Please add a function description */
	UFUNCTION(BlueprintCallable)
	void GiveBuildColor(bool IsAllowed);

	/** Please add a function description */
	UFUNCTION(BlueprintCallable)
	void BuildCycle();

	/** Please add a function description */
	UFUNCTION(BlueprintCallable)
	void ChangeMesh();

	/** Please add a function description */
	UFUNCTION(BlueprintCallable)
	void SpawnBuild(FTransform Transform, int32 ID, AActor* Hit, UPrimitiveComponent* HitComponent);

	/** Please add a function description */
	UFUNCTION(BlueprintCallable)
	void DetectBuildBoxes(bool& OutFound, FTransform& OutTransform);

	/** Please add a function description */
	UFUNCTION(BlueprintCallable)
	bool CheckForOverlap();

	/** Please add a function description */
	UFUNCTION(BlueprintCallable)
	bool IsBuildFloating();

	UFUNCTION()
	void StopBuildMode();

	UFUNCTION()
	void BuildDelay();

public:

	UFUNCTION(BlueprintCallable)
	void LaunchBuildMode();

};
