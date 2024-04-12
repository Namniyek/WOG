// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState_Trail.h"
#include "WOGAnimNotifyAttackSwing.generated.h"

/**
 * 
 */

class UGameplayEffect;
class AWOGBaseCharacter;
class AWOGBaseWeapon;

UCLASS()
class WOG_API UWOGAnimNotifyAttackSwing : public UAnimNotifyState_Trail
{
	GENERATED_BODY()

	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	
	TObjectPtr<AWOGBaseCharacter> OwnerCharacter;
	TObjectPtr<AWOGBaseWeapon> Weapon;

	void StartTrace(AActor* Owner);
	void EndTrace(AActor* Owner);

	void HandleSwingPlayerCharacter();
	void HandleSwingEnemy();

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setup")
	TSubclassOf<class UGameplayEffect> EffectToApply;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setup")
	bool bIsEnemy = false;


};
