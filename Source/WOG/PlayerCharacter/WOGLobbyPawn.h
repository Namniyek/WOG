// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "WOGLobbyPawn.generated.h"

UCLASS()
class WOG_API AWOGLobbyPawn : public APawn
{
	GENERATED_BODY()

public:
	AWOGLobbyPawn();

protected:

	UPROPERTY(EditDefaultsOnly)
	class UCameraComponent* Camera;
};
