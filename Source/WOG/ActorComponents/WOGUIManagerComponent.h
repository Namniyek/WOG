// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WOGUIManagerComponent.generated.h"


class UWOGUIManagerSubsystem;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WOG_API UWOGUIManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UWOGUIManagerComponent();

	friend class AWOGPlayerController;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	TObjectPtr<AWOGPlayerController> OwnerPC;
	TObjectPtr<UWOGUIManagerSubsystem> UIManager;

public:	

	UFUNCTION(BlueprintCallable, Client, unreliable)
	void Client_CreateWarningWidget(const FString& Attribute);
		
};
