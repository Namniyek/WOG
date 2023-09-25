// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameUserSettings.h"
#include "WOGGameUserSettings.generated.h"

/**
 * 
 */
UCLASS()
class WOG_API UWOGGameUserSettings : public UGameUserSettings
{
	GENERATED_UCLASS_BODY()

public:

	#pragma region Setters
	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetAudioMasterVolume(const float& NewVolume) { AudioMasterVolume = NewVolume; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetAudioEffectsVolume(const float& NewVolume) { AudioEffectsVolume = NewVolume; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetAudioVOVolume(const float& NewVolume) { AudioVOVolume = NewVolume; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetAudioMusicVolume(const float& NewVolume) { AudioMusicVolume = NewVolume; }
	#pragma endregion

	#pragma region Getters
	UFUNCTION(BlueprintPure)
	FORCEINLINE float GetAudioMasterVolume() const { return AudioMasterVolume; }
	UFUNCTION(BlueprintPure)
	FORCEINLINE float GetAudioEffectsVolume() const { return AudioEffectsVolume; }
	UFUNCTION(BlueprintPure)
	FORCEINLINE float GetAudioVOVolume() const { return AudioVOVolume; }
	UFUNCTION(BlueprintPure)
	FORCEINLINE float GetAudioMusicVolume() const { return AudioMusicVolume; }
	#pragma endregion

	UFUNCTION(BlueprintCallable)
	static UWOGGameUserSettings* GetWOGGameUserSettings();

protected:
	UPROPERTY(Config)
	float AudioMasterVolume;

	UPROPERTY(Config, meta = (ConsoleVariable = "Audio.Volume.Effects"))
	float AudioEffectsVolume;

	UPROPERTY(Config)
	float AudioVOVolume;

	UPROPERTY(Config)
	float AudioMusicVolume;
};
