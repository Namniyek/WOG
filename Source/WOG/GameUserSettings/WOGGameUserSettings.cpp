// Fill out your copyright notice in the Description page of Project Settings.

#include "GameUserSettings/WOGGameUserSettings.h"

UWOGGameUserSettings::UWOGGameUserSettings(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	AudioMasterVolume = 1.f;
	AudioEffectsVolume = 1.f;
	AudioVOVolume = 1.f;
	AudioMusicVolume = 1.f;
}

UWOGGameUserSettings* UWOGGameUserSettings::GetWOGGameUserSettings()
{
	return CastChecked<UWOGGameUserSettings>(UGameUserSettings::GetGameUserSettings());
}
