// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BasePlayerCharacter.h"
#include "WOGAttacker.generated.h"

/**
 * 
 */
UCLASS()
class WOG_API AWOGAttacker : public ABasePlayerCharacter
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	
};
