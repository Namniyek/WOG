// Copyright BabbyGames All Rights Reserved.

#include "ObjectMarkerBPLibrary.h"
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "SceneView.h"
#include "Blueprint/UserWidget.h"
#include "Components/Slider.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/ActorComponent.h"
#include "UnrealClient.h"
#include "ObjectMarker.h"

UObjectMarkerBPLibrary::UObjectMarkerBPLibrary(const FObjectInitializer& ObjectInitializer) 
: Super(ObjectInitializer)
{

}
UPROPERTY(EditAnywhere)
TArray<AActor*> Actors;

FVector2D UObjectMarkerBPLibrary::ProjectWorldToScreen(APlayerController* PlayerController, const FVector& TargetLocation)
{
	FVector2D ScreenPosition;
	// Project target location to screen space
	if (PlayerController != nullptr && PlayerController->ProjectWorldLocationToScreen(TargetLocation, ScreenPosition, true))
	{
		return ScreenPosition;
	}

	// Return an invalid position if projection fails
	return FVector2D(1, 1);

}

bool UObjectMarkerBPLibrary::CalculateObjectLocationOnScreen(APlayerController* Player, AActor const* TargetActor, FVector2D& ScreenPosition, bool& bTargetBehindCamera, bool bPlayerViewportRelative)
{
	FVector Projected;
	bool bSuccess = false;
	FVector WorldPosition;
	WorldPosition = TargetActor->GetActorLocation();
	ULocalPlayer* const LP = Player ? Player->GetLocalPlayer() : nullptr;
	const FVector2D ViewportSize = FVector2D(GEngine->GameViewport->Viewport->GetSizeXY());
	FVector2D InitialPosition = ProjectWorldToScreen(Player, WorldPosition);
	bool CamBehindObject;
	if (InitialPosition.X > 0 && InitialPosition.Y > 0 && InitialPosition.X < ViewportSize.X && InitialPosition.Y < ViewportSize.Y)
	{
		CamBehindObject = false;
	}
	else
	{
		CamBehindObject = true;
	}
	if (LP && LP->ViewportClient)
	{
		// get the projection data
		FSceneViewProjectionData ProjectionData;
		if (LP->GetProjectionData(LP->ViewportClient->Viewport, /*out*/ ProjectionData))
		{
			const FMatrix ViewProjectionMatrix = ProjectionData.ComputeViewProjectionMatrix();
			const FIntRect ViewRectangle = ProjectionData.GetConstrainedViewRect();
			FPlane Result = ViewProjectionMatrix.TransformFVector4(FVector4(WorldPosition, 1.f));
			if (Result.W < 0.f || CamBehindObject)
			{
				bTargetBehindCamera = true;
			}
			else { bTargetBehindCamera = false; }
			if (Result.W == 0.f) { Result.W = 1.f; } // Prevent Divide By Zero

			const float RHW = 1.f / FMath::Abs(Result.W);
			Projected = FVector(Result.X, Result.Y, Result.Z) * RHW;

			// Normalize to 0..1 UI Space
			const float NormX = (Projected.X / 2.f) + 0.5f;
			const float NormY = 1.f - (Projected.Y / 2.f) - 0.5f;

			Projected.X = (float)ViewRectangle.Min.X + (NormX * (float)ViewRectangle.Width());
			Projected.Y = (float)ViewRectangle.Min.Y + (NormY * (float)ViewRectangle.Height());

			bSuccess = true;
			FVector2D ProjectedPosition;
			ProjectedPosition = FVector2D(Projected.X, Projected.Y);
			FVector2D WidgetSize;
			WidgetSize = FVector2D(128, 128);
			float ScreenWidthRatio = ViewportSize.X / 1920.0f;
			float ScreenHeightRatio = ViewportSize.Y / 1080.0f;

			// Use the ratio to adjust the widget size
			float AdjustedWidth = FMath::GetMappedRangeValueClamped(FVector2D(0.5f, 2.0f), FVector2D(WidgetSize.X / 2.0f, WidgetSize.X * 2.0f), ScreenWidthRatio);
			float AdjustedHeight = FMath::GetMappedRangeValueClamped(FVector2D(0.5f, 2.0f), FVector2D(WidgetSize.Y / 2.0f, WidgetSize.Y * 2.0f), ScreenHeightRatio);

			// Clamp the screen position to keep the widget within screen bounds
			FVector2D AdjustedPosition = ProjectedPosition;
			FVector2D AdjustedWidgetSize;
			AdjustedWidgetSize = FVector2D(AdjustedWidth, AdjustedHeight);
			// Ensure the adjusted position stays within the screen bounds
			AdjustedPosition.X = FMath::Clamp(AdjustedPosition.X, 0.0f, ViewportSize.X - AdjustedWidgetSize.X);
			AdjustedPosition.Y = FMath::Clamp(AdjustedPosition.Y, 0.0f, ViewportSize.Y - AdjustedWidgetSize.Y);
			if (bTargetBehindCamera)
			{

			}
			ScreenPosition = AdjustedPosition;
			ScreenPosition = FVector2D(AdjustedPosition.X, AdjustedPosition.Y);

			if (bPlayerViewportRelative)
			{
				ScreenPosition -= FVector2D(ProjectionData.GetConstrainedViewRect().Min);
			}
		}
		else
		{
			ScreenPosition = FVector2D(1234, 5678);
		}
	}

	return bSuccess;
}

void UObjectMarkerBPLibrary::OffScreenIndicatorController(APlayerController* PlayerController, AActor* TargetActor, USlider* SliderUP, USlider* SliderDOWNRIGHT, USlider* SliderDOWNLEFT, USlider* SliderRIGHT, USlider* SliderLEFT, USlider* SliderDOWN)
{
	FVector CameraLocation = PlayerController->PlayerCameraManager->GetCameraLocation();
	FVector ActorLocation = TargetActor->GetActorLocation();
	FRotator CameraRotation = PlayerController->PlayerCameraManager->GetCameraRotation();
	FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(CameraLocation, ActorLocation);
	float ValueYAW = UKismetMathLibrary::NormalizedDeltaRotator(CameraRotation, LookAtRotation).Yaw * -1;
	float ValuePITCH = UKismetMathLibrary::NormalizedDeltaRotator(CameraRotation, LookAtRotation).Pitch;
	bool IsTargetDownSideOfTheCamera = false;
	if (ValueYAW >= -44 && ValueYAW <= 44 && ValuePITCH >= 30)
	{
		SliderUP->SetVisibility(ESlateVisibility::Hidden);
		SliderDOWNRIGHT->SetVisibility(ESlateVisibility::Hidden);
		SliderDOWNLEFT->SetVisibility(ESlateVisibility::Hidden);
		SliderRIGHT->SetVisibility(ESlateVisibility::Hidden);
		SliderLEFT->SetVisibility(ESlateVisibility::Hidden);
		SliderDOWN->SetVisibility(ESlateVisibility::Visible);
		SliderDOWN->SetValue(ValueYAW * -1);
		IsTargetDownSideOfTheCamera = true;
	}
	if (ValueYAW >= -44 && ValueYAW <= 44 && !IsTargetDownSideOfTheCamera)
	{
		SliderUP->SetVisibility(ESlateVisibility::Visible);
		SliderDOWNRIGHT->SetVisibility(ESlateVisibility::Hidden);
		SliderDOWNLEFT->SetVisibility(ESlateVisibility::Hidden);
		SliderRIGHT->SetVisibility(ESlateVisibility::Hidden);
		SliderLEFT->SetVisibility(ESlateVisibility::Hidden);
		SliderDOWN->SetVisibility(ESlateVisibility::Hidden);
		SliderUP->SetValue(ValueYAW);
	}
	if (ValueYAW >= 45 && ValueYAW <= 120)
	{
		SliderUP->SetVisibility(ESlateVisibility::Hidden);
		SliderDOWNRIGHT->SetVisibility(ESlateVisibility::Hidden);
		SliderDOWNLEFT->SetVisibility(ESlateVisibility::Hidden);
		SliderRIGHT->SetVisibility(ESlateVisibility::Visible);
		SliderLEFT->SetVisibility(ESlateVisibility::Hidden);
		SliderDOWN->SetVisibility(ESlateVisibility::Hidden);
		SliderRIGHT->SetValue(ValuePITCH);
	}
	if (ValueYAW >= 121 && ValueYAW <= 181)
	{
		SliderUP->SetVisibility(ESlateVisibility::Hidden);
		SliderDOWNRIGHT->SetVisibility(ESlateVisibility::Visible);
		SliderDOWNLEFT->SetVisibility(ESlateVisibility::Hidden);
		SliderRIGHT->SetVisibility(ESlateVisibility::Hidden);
		SliderLEFT->SetVisibility(ESlateVisibility::Hidden);
		SliderDOWN->SetVisibility(ESlateVisibility::Hidden);
		SliderDOWNRIGHT->SetValue(ValueYAW);
	}
	if (ValueYAW >= -180 && ValueYAW <= -121)
	{
		SliderUP->SetVisibility(ESlateVisibility::Hidden);
		SliderDOWNRIGHT->SetVisibility(ESlateVisibility::Hidden);
		SliderDOWNLEFT->SetVisibility(ESlateVisibility::Visible);
		SliderRIGHT->SetVisibility(ESlateVisibility::Hidden);
		SliderLEFT->SetVisibility(ESlateVisibility::Hidden);
		SliderDOWN->SetVisibility(ESlateVisibility::Hidden);
		SliderDOWNLEFT->SetValue(ValueYAW);
	}
	if (ValueYAW >= -120 && ValueYAW <= -45)
	{
		SliderUP->SetVisibility(ESlateVisibility::Hidden);
		SliderDOWNRIGHT->SetVisibility(ESlateVisibility::Hidden);
		SliderDOWNLEFT->SetVisibility(ESlateVisibility::Hidden);
		SliderRIGHT->SetVisibility(ESlateVisibility::Hidden);
		SliderLEFT->SetVisibility(ESlateVisibility::Visible);
		SliderDOWN->SetVisibility(ESlateVisibility::Hidden);
		SliderLEFT->SetValue(-1 * ValuePITCH);
	}
}

void UObjectMarkerBPLibrary::AddIndicatedObjectsToArray(AActor* Actor)
{
	Actors.Add(Actor);
}

void UObjectMarkerBPLibrary::GetIndicatedActors(TArray<AActor*>& MarkedActors)
{
	MarkedActors = Actors;
}

void UObjectMarkerBPLibrary::ResetObjectIndicator()
{
	Actors.Empty();
}
