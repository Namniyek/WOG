// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGAnimNotifyAttackSwing.h"
#include "WOG/PlayerCharacter/BasePlayerCharacter.h"
#include "WOG/ActorComponents/WOGCombatComponent.h"
#include "WOG/Weapons/WOGBaseWeapon.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "DidItHitActorComponent.h"

void UWOGAnimNotifyAttackSwing::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (!MeshComp || !MeshComp->GetOwner()) return;

	OwnerCharacter = Cast<ABasePlayerCharacter>(MeshComp->GetOwner());
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid OwnerCharacter"));
		return;
	}
	Weapon = OwnerCharacter->GetCombatComponent()->GetEquippedWeapon();
	if (!Weapon)
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid Weapon"));
		return;
	}
	if (Weapon->GetSwingSound())
	{
		UGameplayStatics::PlaySoundAtLocation(Weapon, Weapon->GetSwingSound(), Weapon->GetActorLocation());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid SwingSound"));
		return;
	}

	if(Weapon->GetTraceComponent())
	{
		Weapon->GetTraceComponent()->ToggleTraceCheck(true);
	}
}

void UWOGAnimNotifyAttackSwing::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);
}

void UWOGAnimNotifyAttackSwing::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	OwnerCharacter = OwnerCharacter == nullptr ? Cast<ABasePlayerCharacter>(MeshComp->GetOwner()) : OwnerCharacter;
	if (!OwnerCharacter) return;
	Weapon = Weapon == nullptr ? OwnerCharacter->GetCombatComponent()->GetEquippedWeapon() : Weapon;
	if (!Weapon) return;

	if (Weapon->GetTraceComponent())
	{
		Weapon->GetTraceComponent()->ToggleTraceCheck(false);
	}
}
