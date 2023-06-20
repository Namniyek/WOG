// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGBaseEnemy.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AIController.h"
#include "WOG/Weapons/WOGBaseWeapon.h"
#include "Kismet/GameplayStatics.h"
#include "ActorComponents/WOGAbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffectExtension.h"
#include "Types/WOGGameplayTags.h"
#include "Components/CapsuleComponent.h"

// Sets default values
AWOGBaseEnemy::AWOGBaseEnemy()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

}

void AWOGBaseEnemy::BeginPlay()
{
	Super::BeginPlay();
}

void AWOGBaseEnemy::ProcessHit(FHitResult Hit, UPrimitiveComponent* WeaponMesh)
{

}

void AWOGBaseEnemy::BroadcastHit_Implementation(AActor* AgressorActor, const FHitResult& Hit, const float& DamageToApply, AActor* InstigatorWeapon)
{
	if (HasMatchingGameplayTag(TAG_State_Dead)) return;
	if (HasMatchingGameplayTag(TAG_State_Dodging)) return;

	//Store the last hit result
	LastHitResult = Hit;

	UE_LOG(LogTemp, Warning, TEXT("%s damaged with %s, by %s, in the amount : %f"), *GetNameSafe(Hit.GetActor()), *GetNameSafe(InstigatorWeapon), *GetNameSafe(AgressorActor), DamageToApply);

	//Apply HitReact or KO to character
	TObjectPtr<AWOGBaseCharacter> AgressorCharacter = Cast<AWOGBaseCharacter>(AgressorActor);
	if (AgressorCharacter && AgressorCharacter->HasMatchingGameplayTag(TAG_State_Weapon_AttackHeavy) && IsHitFrontal(60.f, this, InstigatorWeapon))
	{
		//Send event KO
		FGameplayEventData EventKOPayload;
		EventKOPayload.EventTag = TAG_Event_Debuff_KO;
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, TAG_Event_Debuff_KO, EventKOPayload);
		UE_LOG(LogTemp, Warning, TEXT("Enemy KO Applied : %s"), *UEnum::GetValueAsString(GetLocalRole()));
	}
	else
	{
		//Send Event light HitReact 
		FGameplayEventData EventHitReactPayload;
		EventHitReactPayload.EventTag = TAG_Event_Debuff_HitReact;
		EventHitReactPayload.Instigator = InstigatorWeapon;
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, TAG_Event_Debuff_HitReact, EventHitReactPayload);
		UE_LOG(LogTemp, Warning, TEXT("Enemy Hit react applied : %s"), *UEnum::GetValueAsString(GetLocalRole()));
	}

	//Apply damage to character if authority
	TObjectPtr<AWOGBaseWeapon> Weapon = Cast<AWOGBaseWeapon>(InstigatorWeapon);
	if (HasAuthority() && AgressorCharacter && AbilitySystemComponent.Get() && Weapon && Weapon->GetWeaponData().WeaponDamageEffect)
	{
		FGameplayEffectContextHandle DamageContext = AbilitySystemComponent.Get()->MakeEffectContext();
		DamageContext.AddInstigator(AgressorCharacter, Weapon);
		DamageContext.AddHitResult(Hit);

		FGameplayEffectSpecHandle OutSpec = AbilitySystemComponent->MakeOutgoingSpec(Weapon->GetWeaponData().WeaponDamageEffect, 1, DamageContext);
		UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(OutSpec, FGameplayTag::RequestGameplayTag(TEXT("Damage.Attribute.Health")), -DamageToApply);
		AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*OutSpec.Data);
	}
}

void AWOGBaseEnemy::HandleStateElimmed(AController* InstigatedBy)
{
	Elim(false);
}

void AWOGBaseEnemy::Elim(bool bPlayerLeftGame)
{
	Multicast_Elim(bPlayerLeftGame);
}

void AWOGBaseEnemy::Multicast_Elim_Implementation(bool bPlayerLeftGame)
{
	GetMesh()->SetCollisionProfileName(FName("Ragdoll"));
	GetMesh()->SetAllBodiesSimulatePhysics(true);
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FVector ImpulseDirection = LastHitDirection.GetSafeNormal() * -65000.f;
	GetMesh()->AddImpulse(ImpulseDirection);

	/*
	**Handle destroy timer
	*/
	GetWorld()->GetTimerManager().SetTimer(ElimTimer, this, &ThisClass::ElimTimerFinished, ElimDelay);
}

void AWOGBaseEnemy::ElimTimerFinished()
{
	Destroy();
}

