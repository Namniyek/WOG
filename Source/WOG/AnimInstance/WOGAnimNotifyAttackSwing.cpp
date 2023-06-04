// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGAnimNotifyAttackSwing.h"
#include "WOG/PlayerCharacter/BasePlayerCharacter.h"
#include "WOG/Weapons/WOGBaseWeapon.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Libraries/WOGBlueprintLibrary.h"
#include "Data/AGRLibrary.h"

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

	Weapon = UWOGBlueprintLibrary::GetEquippedWeapon(OwnerCharacter);
	if (!Weapon)
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid Weapon"));
		return;
	}
	if (Weapon->GetWeaponData().SwingSound)
	{
		UGameplayStatics::PlaySoundAtLocation(Weapon, Weapon->GetWeaponData().SwingSound, Weapon->GetActorLocation());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid SwingSound"));
		return;
	}

	StartTrace(OwnerCharacter);
}

void UWOGAnimNotifyAttackSwing::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (MeshComp->GetOwner())
	{
		EndTrace(MeshComp->GetOwner());
	}

	Super::NotifyEnd(MeshComp, Animation, EventReference);
}

void UWOGAnimNotifyAttackSwing::StartTrace(AActor* Owner)
{
	if (!Owner) return;
	if (!OwnerCharacter/* || (!OwnerCharacter->HasAuthority() && !OwnerCharacter->IsLocallyControlled())*/) return;

	UAGR_CombatManager* CombatManager = UAGRLibrary::GetCombatManager(Owner);
	if (CombatManager)
	{
		CombatManager->StartTrace();
	}
}

void UWOGAnimNotifyAttackSwing::EndTrace(AActor* Owner)
{
	if (!Owner) return;
	if (!OwnerCharacter /*|| (!OwnerCharacter->HasAuthority() && !OwnerCharacter->IsLocallyControlled())*/) return;

	UAGR_CombatManager* CombatManager = UAGRLibrary::GetCombatManager(Owner);
	if (CombatManager)
	{
		CombatManager->EndTrace();
	}
}

void UWOGAnimNotifyAttackSwing::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);
}