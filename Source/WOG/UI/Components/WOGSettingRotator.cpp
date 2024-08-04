// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Components/WOGSettingRotator.h"
#include "Components/NamedSlot.h"

bool UWOGSettingRotator::Initialize()
{
	if (Super::Initialize())
	{
		OnNavigation.BindUObject(this, &ThisClass::HandleNavigation);

		return true;
	}

	return false;
}

FNavigationReply UWOGSettingRotator::NativeOnNavigation(const FGeometry& MyGeometry,
                                                        const FNavigationEvent& InNavigationEvent, const FNavigationReply& InDefaultReply)
{
	switch (InNavigationEvent.GetNavigationType())
	{
	case EUINavigation::Left:
	case EUINavigation::Right:
		return FNavigationReply::Custom(OnNavigation);
	default:
		return InDefaultReply;
	}
}

TSharedPtr<SWidget> UWOGSettingRotator::HandleNavigation(EUINavigation UINavigation)
{
	if (UINavigation == EUINavigation::Left)
	{
		RotateLeftInternal(true);
	}
	else if (UINavigation == EUINavigation::Right)
	{
		RotateRightInternal(true);
	}

	return nullptr;
}

void UWOGSettingRotator::RotateLeft()
{
	RotateLeftInternal(false);
}

void UWOGSettingRotator::RotateRight()
{
	RotateRightInternal(false);
}

void UWOGSettingRotator::RotateLeftInternal(bool bFromNavigation) const
{
	if(SettingSlot && SettingSlot->GetChildAt(0) && SettingSlot->GetChildAt(0)->Implements<UUIInterface>())
	{
		Execute_SelectPrevious(SettingSlot->GetChildAt(0));
	}
	
	if(OnRotatedLeft.IsBound())
	{
		OnRotatedLeft.Broadcast();
	}
}

void UWOGSettingRotator::RotateRightInternal(bool bFromNavigation) const
{
	if(SettingSlot && SettingSlot->GetChildAt(0) && SettingSlot->GetChildAt(0)->Implements<UUIInterface>())
	{
		Execute_SelectNext(SettingSlot->GetChildAt(0));
	}
	
	if(OnRotatedRight.IsBound())
	{
		OnRotatedRight.Broadcast();
	}
}
