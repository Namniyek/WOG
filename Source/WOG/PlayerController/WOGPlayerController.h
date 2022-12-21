// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "WOGPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class WOG_API AWOGPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	int32 UserIndex;

protected:
	virtual void OnPossess(APawn* aPawn) override;

private:
	UFUNCTION(Server, reliable)
	void Server_SetPlayerIndex(int32 NewIndex);


	
};
