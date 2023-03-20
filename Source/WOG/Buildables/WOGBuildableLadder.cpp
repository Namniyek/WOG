// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGBuildableLadder.h"
#include "Components/SceneComponent.h"
#include "Kismet/KismetMathLibrary.h"

void AWOGBuildableLadder::SetProperties_Implementation(UStaticMesh* Mesh, UStaticMesh* ExtensionMesh, const float& Health, const float& MaxHeightOffset)
{
	GetStaticMeshComponent()->SetStaticMesh(Mesh);
}

void AWOGBuildableLadder::ExtendLadder()
{
	TObjectPtr<USceneComponent> FirstAnchor = Cast<USceneComponent>(GetComponentByClass(USceneComponent::StaticClass()));

	if (!FirstAnchor)
	{
		UE_LOG(LogTemp, Error, TEXT("FirstAnchor invalid"));
		return;
	}

	for (int32 i = 1; i < 7; i++)
	{
		FHitResult HitResult;
		FTransform FirstAnchorTransform = FirstAnchor->GetComponentTransform();
		FVector Start = UKismetMathLibrary::TransformLocation(FirstAnchorTransform, FVector(0.f, ((i - 1) * -100), ((i - 1) * -100)));
		FVector End = Start - FVector(0, 0, 600);

		bool bHit = GetWorld()->LineTraceSingleByChannel(
						HitResult,
						Start,
						End,
						ECollisionChannel::ECC_Visibility
		);

		if (!bHit)
		{
			UE_LOG(LogTemp, Error, TEXT("bHit false"));
			return;
		}

		if (bHit && HitResult.Distance < 10)
		{
			UE_LOG(LogTemp, Error, TEXT("bHit true, too short distance"));
			return;
		}

		FTransform RelativeTransform = FTransform();
		FVector RelativeLocation = FVector(0, (-i * 100), (-i * 100));
		RelativeTransform.SetLocation(RelativeLocation);

		TObjectPtr<UStaticMeshComponent> Extension =
			Cast<UStaticMeshComponent>(AddComponentByClass(UStaticMeshComponent::StaticClass(), false, RelativeTransform, false));
		if (Extension)
		{
			BuildExtensionsMeshes.AddUnique(Extension);
			Extension->SetStaticMesh(GetStaticMeshComponent()->GetStaticMesh());
		}
	}
}
