// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WOGBaseActorComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WOG_API UWOGBaseActorComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	friend class AWOGBaseCharacter;
	UWOGBaseActorComponent();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(Replicated)
	TObjectPtr<AWOGBaseCharacter> OwnerCharacter;

	UPROPERTY()
	class AWOGPlayerController* OwnerPC;

public:	
		
};
