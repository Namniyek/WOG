// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "WOGEpicOnlineServicesSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class WOG_API UWOGEpicOnlineServicesSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

protected:
	UFUNCTION(BlueprintCallable)
	void Login();

private:
	
	#pragma region Callback functions
	void HandleLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId &UserId, const FString &Error);
	#pragma endregion

	#pragma region Delegate handles
	FDelegateHandle LoginDelegateHandle;
	#pragma endregion 
	
};
