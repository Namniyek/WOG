// Copyright Sam Bonifacio. All Rights Reserved.

#include "UI/SettingControls/NativeSliderSetting.h"

#include <string>

void UNativeSliderSetting::NativeConstruct()
{
	Super::NativeConstruct();

	if (IsValid(Slider))
	{
		Slider->OnValueChanged.AddUniqueDynamic(this, &UNativeSliderSetting::SliderValueChanged);
		Slider->OnMouseCaptureBegin.AddUniqueDynamic(this, &UNativeSliderSetting::SliderMouseCaptureBegin);
		Slider->OnMouseCaptureEnd.AddUniqueDynamic(this, &UNativeSliderSetting::SliderMouseCaptureEnd);
	}
}

void UNativeSliderSetting::UpdateSliderValue_Implementation(float NormalizedValue, float RawValue)
{
	if (IsValid(Slider))
		Slider->SetValue(NormalizedValue);
}

void UNativeSliderSetting::SliderValueChanged(float NewValue)
{
	SliderValueUpdated(NewValue);
}

void UNativeSliderSetting::SliderMouseCaptureBegin()
{
	bMouseCaptureInProgress = true;
}

void UNativeSliderSetting::SliderMouseCaptureEnd()
{
	bMouseCaptureInProgress = false;

	// Call SliderValueUpdated after released to trigger a save if we need one
	SliderValueUpdated(Slider->GetValue());
}

void UNativeSliderSetting::PreviousValue(const float Increment)
{
	
	Slider->SetValue(FMath::Clamp(Slider->GetValue()-Increment, LeftValue, RightValue));
}

void UNativeSliderSetting::NextValue(const float Increment)
{
	Slider->SetValue(FMath::Clamp(Slider->GetValue()+Increment, LeftValue, RightValue));
}
