// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "WOGGameInstance.generated.h"

USTRUCT(BlueprintType)
struct FFriendItem
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
		FString FriendName = FString("");
	UPROPERTY(BlueprintReadOnly)
		int32 bIsOnline = 0;
	UPROPERTY(BlueprintReadOnly)
		int32 bIsPlayingThisGame = 0;
	UPROPERTY(BlueprintReadOnly)
		FString UniqueID = FString("");;
	UPROPERTY(BlueprintReadOnly)
		int32 ServerArrayIndex = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFriendFoundDelegate, FFriendItem, FriendInfo);

/**
 *
 */
UCLASS()
class WOG_API UWOGGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UWOGGameInstance();
	void ClearPlayerMap();

	virtual void Init() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<int, FString> PlayersMap;


protected:
	class IOnlineSubsystem* OnlineSubsystem;

	bool bIsLoggedIn;

	UPROPERTY(BlueprintAssignable)
	FFriendFoundDelegate FriendFoundDelegate;

public:
	UFUNCTION(BlueprintCallable)
	void GetFriends();
	void OnGetFriendsComplete(int32 LocalUserNum, bool bWasSuccessful, const FString& ListName, const FString& ErrorStr);

	UFUNCTION(BlueprintCallable)
	bool SendInvite(FString UniqueID);
	
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);


};