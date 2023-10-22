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
	FString PlayerName;

	UPROPERTY(BlueprintReadWrite)
	int32 UserIndex;

	UPROPERTY(BlueprintReadWrite)
	bool bIsAttacker;

	UPROPERTY(BlueprintReadWrite)
	bool bIsMale;

	UPROPERTY(BlueprintReadWrite)
	FName CharacterIndex;

	UPROPERTY(BlueprintReadWrite)
	FName PrimaryColor;

	UPROPERTY(BlueprintReadWrite)
	FName SecondaryColor;

	UPROPERTY(BlueprintReadWrite)
	FName SkinColor;

	UPROPERTY(BlueprintReadWrite)
	FName BodyPaintColor;

	UPROPERTY(BlueprintReadWrite)
	FName HairColor;

	UPROPERTY(BlueprintReadWrite)
	FName Rune;

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
