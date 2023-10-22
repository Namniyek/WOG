// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState_Trail.h"
#include "GameplayEffectTypes.h"
#include "WOGAnimNotifyAttackSwing.generated.h"

/**
 * 
 */
UCLASS()
class WOG_API UWOGAnimNotifyAttackSwing : public UAnimNotifyState_Trail
{
	GENERATED_BODY()

	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	
	TObjectPtr<class ABasePlayerCharacter> OwnerCharacter;
	TObjectPtr<class AWOGBaseWeapon> Weapon;

	void StartTrace(AActor* Owner);
	void EndTrace(AActor* Owner);

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameplayEffect")
	TSubclassOf<class UGameplayEffect> EffectToApply;


};
