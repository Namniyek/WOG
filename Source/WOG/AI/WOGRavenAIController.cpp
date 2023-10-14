// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/WOGRavenAIController.h"
#include "Enemies/WOGRaven.h"
#include "Utilities/WOGSpline.h"
#include "Kismet/GameplayStatics.h"
#include "CPathVolume.h"
#include "Components/SplineComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/PawnMovementComponent.h"


void AWOGRavenAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (!InPawn) return;
	Raven = Cast<AWOGRaven>(InPawn);

	if (!Raven) return;
	RavenSpline = Raven->GetAssignedSplinePath();
	RavenMovement = Raven->GetMovementComponent();

	NavVolumeRef = Cast<ACPathVolume>(UGameplayStatics::GetActorOfClass(this, ACPathVolume::StaticClass()));
}

void AWOGRavenAIController::Tick(float DeltaTime)
{
	if (!Raven) return;

	if (bIsOnSpline)
	{
		FollowRavenSpline();
		return;
	}
	else if(RavenMovement)
	{
		RavenMovement->AddInputVector(CurrentInputVector, false);
		return;
	}
}

void AWOGRavenAIController::StartRavenMovement_Implementation()
{
	FindClosestPoint();
}

void AWOGRavenAIController::FindClosestPoint()
{
	if (!Raven || !RavenSpline || !RavenSpline->GetSplineComponent()) return;

	Destination = RavenSpline->GetSplineComponent()->FindLocationClosestToWorldLocation(Raven->GetActorLocation(), ESplineCoordinateSpace::World);

	if (bDrawDebug)
	{
		DrawDebugSphere(GetWorld(), Destination, 100.f, 12, FColor::Red, false, 60.f);
	}
}

void AWOGRavenAIController::FollowRavenSpline()
{
	if (!Raven || !RavenSpline || !RavenSpline->GetSplineComponent() || !RavenMovement) return;
	
	FVector TempLocation = RavenSpline->GetSplineComponent()->FindLocationClosestToWorldLocation(Raven->GetActorLocation(), ESplineCoordinateSpace::World);
	FRotator TempRotation = RavenSpline->GetSplineComponent()->FindRotationClosestToWorldLocation(Raven->GetActorLocation(), ESplineCoordinateSpace::World);

	Raven->SetActorLocationAndRotation(TempLocation, TempRotation);

	FVector TempInputVector = RavenSpline->GetSplineComponent()->FindTangentClosestToWorldLocation(Raven->GetActorLocation(), ESplineCoordinateSpace::World);
	RavenMovement->AddInputVector(TempInputVector);
}
