// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGBaseEnemyAIController.h"

void AWOGBaseEnemyAIController::BeginPlay()
{
	Super::BeginPlay();

	if (BaseEnemyBehaviorTree)
	{
		RunBehaviorTree(BaseEnemyBehaviorTree);
		UE_LOG(LogTemp, Warning, TEXT("Behaviour tree is running"));
	}
}
