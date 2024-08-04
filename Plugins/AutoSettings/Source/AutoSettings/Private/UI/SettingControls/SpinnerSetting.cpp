// Copyright Sam Bonifacio. All Rights Reserved.

#include "UI/SettingControls/SpinnerSetting.h"

void USpinnerSetting::SelectNextOption()
{
	if (IsValid(Spinner))
		Spinner->Next();
}

void USpinnerSetting::SelectPreviousOption()
{
	if (IsValid(Spinner))
		Spinner->Previous();
}

void USpinnerSetting::NativeConstruct()
{
	Super::NativeConstruct();
	
	if(IsValid(Spinner))
		Spinner->SelectionChangedEvent.AddUniqueDynamic(this, &USpinnerSetting::SpinnerSelectionChanged);
	
	if(!bUseCVar && Spinner && !Options.IsEmpty())
	{
		Spinner->SelectIndex(0);
	}
}

void USpinnerSetting::UpdateSelection_Implementation(const FString & Value)
{
	if (IsValid(Spinner))
		Spinner->SelectValue(Value);
	
}

void USpinnerSetting::UpdateOptions_Implementation(const TArray<FSettingOption>& InOptions)
{
	if (IsValid(Spinner))
		Spinner->Options = InOptions;
}

void USpinnerSetting::SpinnerSelectionChanged(FString Value)
{
	ApplySettingValue(Value);
	OnSelectionChanged.Broadcast(Value);
}
