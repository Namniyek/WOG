// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WOGScreenDamage.h"

void UWOGScreenDamage::SetRadiusValue(const int32& DamageThreshold)
{
	switch (DamageThreshold)
	{
	case 0:
		RadiusMin = 0.65f;
		break;
	case 1:
		RadiusMin = 0.55f;
		break;
	case 2:
		RadiusMin = 0.45f;
		break;
	default:
		break;
	}
}
