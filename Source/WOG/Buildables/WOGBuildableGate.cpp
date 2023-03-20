// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGBuildableGate.h"


void AWOGBuildableGate::SetProperties_Implementation(UStaticMesh* Mesh, UStaticMesh* ExtensionMesh, const float& Health, const float& MaxHeightOffset)
{
	BuildHealth = Health;
	BuildMaxHeightOffset = MaxHeightOffset;
}
