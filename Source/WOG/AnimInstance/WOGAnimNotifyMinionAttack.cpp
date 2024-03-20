// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimInstance/WOGAnimNotifyMinionAttack.h"
#include "Enemies/WOGBaseEnemy.h"
#include "Data/AGRLibrary.h"

void UWOGAnimNotifyMinionAttack::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	GEngine->AddOnScreenDebugMessage(1, 2, FColor::Emerald, FString("NotifyBegin() called"));

	if (!MeshComp || !MeshComp->GetOwner()) return;

	Minion = Cast<AWOGBaseEnemy>(MeshComp->GetOwner());
	if (!Minion) return;

	StartTrace(Minion);

}

void UWOGAnimNotifyMinionAttack::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (MeshComp->GetOwner())
	{
		EndTrace(MeshComp->GetOwner());
	}

	Super::NotifyEnd(MeshComp, Animation, EventReference);
}

void UWOGAnimNotifyMinionAttack::StartTrace(AActor* Owner)
{
	if (!Owner) return;

	UAGR_CombatManager* CombatManager = UAGRLibrary::GetCombatManager(Owner);
	if (CombatManager)
	{
		CombatManager->StartTrace();
	}
}

void UWOGAnimNotifyMinionAttack::EndTrace(AActor* Owner)
{
	if (!Owner) return;

	UAGR_CombatManager* CombatManager = UAGRLibrary::GetCombatManager(Owner);
	if (CombatManager)
	{
		CombatManager->EndTrace();
	}
}
