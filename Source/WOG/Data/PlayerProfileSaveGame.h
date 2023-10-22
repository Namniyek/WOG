// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "PlayerProfileSaveGame.generated.h"

USTRUCT(BlueprintType)
struct FPlayerData
{

	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	FString PlayerName = FString("");;

	UPROPERTY(BlueprintReadWrite)
	int32 UserIndex = 0;

	UPROPERTY(BlueprintReadWrite)
	bool bIsAttacker = false;

	UPROPERTY(BlueprintReadWrite)
	bool bIsMale = false;

	UPROPERTY(BlueprintReadWrite)
	FName CharacterIndex = FName("");

	UPROPERTY(BlueprintReadWrite)
	FName PrimaryColor = FName("");

	UPROPERTY(BlueprintReadWrite)
	FName SecondaryColor = FName("");

	UPROPERTY(BlueprintReadWrite)
	FName SkinColor = FName("");

	UPROPERTY(BlueprintReadWrite)
	FName BodyPaintColor = FName("");

	UPROPERTY(BlueprintReadWrite)
	FName HairColor = FName("");

	UPROPERTY(BlueprintReadWrite)
	FName Rune = FName("");

};
/**
 * 
 */
UCLASS()
class WOG_API UPlayerProfileSaveGame : public USaveGame
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadWrite)
	FPlayerData PlayerProfile;
};
