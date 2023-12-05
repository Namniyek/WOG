// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/WOGBaseDayNPCAIController.h"
#include "Navigation/CrowdFollowingComponent.h"

AWOGBaseDayNPCAIController::AWOGBaseDayNPCAIController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UCrowdFollowingComponent>(TEXT("PathFollowingComponent")))
{

}