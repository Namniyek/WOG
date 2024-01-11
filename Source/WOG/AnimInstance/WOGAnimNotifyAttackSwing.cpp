// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGAnimNotifyAttackSwing.h"
#include "WOG.h"
#include "WOG/PlayerCharacter/BasePlayerCharacter.h"
#include "Characters//WOGBaseCharacter.h"
#include "WOG/Weapons/WOGBaseWeapon.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Libraries/WOGBlueprintLibrary.h"
#include "Data/AGRLibrary.h"
#include "AbilitySystemComponent.h"
#include "Data/WOGGameplayTags.h"
#include "GameplayEffect.h"

void UWOGAnimNotifyAttackSwing::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (!MeshComp || !MeshComp->GetOwner()) return;

	OwnerCharacter = Cast<AWOGBaseCharacter>(MeshComp->GetOwner());
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid OwnerCharacter"));
		return;
	}

	if (!bIsEnemy)
	{
		HandleSwingPlayerCharacter();
	}
	else
	{
		HandleSwingEnemy();
	}

	if (OwnerCharacter->GetAbilitySystemComponent() && EffectToApply && OwnerCharacter->HasAuthority())
	{
		FGameplayEffectContextHandle EffectContext = OwnerCharacter->GetAbilitySystemComponent()->MakeEffectContext();

		FGameplayEffectSpecHandle OutSpec = OwnerCharacter->GetAbilitySystemComponent()->MakeOutgoingSpec(EffectToApply, 1, EffectContext);
		OwnerCharacter->GetAbilitySystemComponent()->ApplyGameplayEffectSpecToSelf(*OutSpec.Data);
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

	UAGR_CombatManager* CombatManager = UAGRLibrary::GetCombatManager(Owner);
	if (CombatManager)
	{
		CombatManager->StartTrace();
	}
}

void UWOGAnimNotifyAttackSwing::EndTrace(AActor* Owner)
{
	if (!Owner) return;

	UAGR_CombatManager* CombatManager = UAGRLibrary::GetCombatManager(Owner);
	if (CombatManager)
	{
		CombatManager->EndTrace();
	}
}

void UWOGAnimNotifyAttackSwing::HandleSwingPlayerCharacter()
{
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
}

void UWOGAnimNotifyAttackSwing::HandleSwingEnemy()
{
	
}

void UWOGAnimNotifyAttackSwing::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);
}