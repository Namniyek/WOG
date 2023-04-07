// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGBaseEnemyAIController.h"

void AWOGBaseEnemyAIController::BeginPlay()
{
	Super::BeginPlay();
}

void AWOGBaseEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (BaseEnemyBehaviorTree)
	{
		RunBehaviorTree(BaseEnemyBehaviorTree);
	}
}
