// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WOG/Characters/WOGBaseCharacter.h"
#include "WOG/Interfaces/AttributesInterface.h"
#include "WOG/Interfaces/SpawnInterface.h"
#include "WOGBaseEnemy.generated.h"

UCLASS()
class WOG_API AWOGBaseEnemy : public AWOGBaseCharacter, public IAttributesInterface, public ISpawnInterface
{
	GENERATED_BODY()

public:
	AWOGBaseEnemy();

protected:
	virtual void BeginPlay() override;

	#pragma region Actor Components

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<class ULockOnTargetComponent> LockOnTarget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<class UTargetingHelperComponent> TargetAttractor;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<class UWOGAttributesComponent> Attributes;

	#pragma endregion

	#pragma region Interface Functions
	

	#pragma endregion

public:	
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	FORCEINLINE UWOGAttributesComponent* GetAttributes() const { return Attributes; }

};
