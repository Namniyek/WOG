// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "WOGBaseEnemyAIController.generated.h"

/**
 * 
 */
UCLASS()
class WOG_API AWOGBaseEnemyAIController : public AAIController
{
	GENERATED_BODY()

protected:

	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<class UBehaviorTree> BaseEnemyBehaviorTree;
	
};
