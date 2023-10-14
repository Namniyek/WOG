// Fill out your copyright notice in the Description page of Project Settings.


#include "Utilities/WOGSpline.h"
#include "Components/SplineComponent.h"

AWOGSpline::AWOGSpline()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;

	SplineComponent = CreateDefaultSubobject<USplineComponent>(TEXT("Spline Component"));
	SetRootComponent(SplineComponent);
}


