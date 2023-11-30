// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState_Trail.h"
#include "WOGAnimNotifyMinionAttack.generated.h"

/**
 * 
 */
UCLASS()
class WOG_API UWOGAnimNotifyMinionAttack : public UAnimNotifyState_Trail
{
	GENERATED_BODY()
	
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	TObjectPtr<class AWOGBaseEnemy> Minion;

	void StartTrace(AActor* Owner);
	void EndTrace(AActor* Owner);
};
