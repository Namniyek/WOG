// Fill out your copyright notice in the Description page of Project Settings.


#include "AssetManager/WOGAssetManager.h"
#include "AbilitySystemGlobals.h"

UWOGAssetManager& UWOGAssetManager::Get()
{
	UWOGAssetManager* Singleton = Cast<UWOGAssetManager>(GEngine->AssetManager);

	if (Singleton)
	{
		return *Singleton;
	}
	else
	{
		UE_LOG(LogTemp, Fatal, TEXT("Invalid AssetManager in DefaultEngine.ini, must be GDAssetManager!"));
		return *NewObject<UWOGAssetManager>();	 // never calls this
	}
}

void UWOGAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();
	UAbilitySystemGlobals::Get().InitGlobalData();
}
