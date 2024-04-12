// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WOGRavenMarkerWidget.h"

void UWOGRavenMarkerWidget::SetAmountAvailableMarkers_Implementation(const int32& Amount)
{
	switch (Amount)
	{
	case 0:
		AmountAvailableMarkers = 3;
		break;
	case 1:
		AmountAvailableMarkers = 2;
		break;
	case 2:
		AmountAvailableMarkers = 1;
		break;
	case 3:
		AmountAvailableMarkers = 0;
	default:
		break;
	}
}
