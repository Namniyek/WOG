// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGBaseEnemy.h"
#include "WOG.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AIController.h"
#include "WOG/Weapons/WOGBaseWeapon.h"
#include "Kismet/GameplayStatics.h"
#include "ActorComponents/WOGAbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffectExtension.h"
#include "Data/WOGGameplayTags.h"
#include "Components/CapsuleComponent.h"
#include "PlayerCharacter/WOGAttacker.h"
#include "AI/Combat/WOGBaseSquad.h"

AWOGBaseEnemy::AWOGBaseEnemy()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	CombatManager = CreateDefaultSubobject<UAGR_CombatManager>(TEXT("CombatManager"));
	CombatManager->SetIsReplicated(true);
	CombatManager->OnStartAttack.AddDynamic(this, &ThisClass::OnStartAttack);
	CombatManager->OnAttackHitEvent.AddDynamic(this, &ThisClass::OnAttackHit);

	CharacterData.bIsAttacker = true;

	bUseControllerRotationPitch = true;
	bUseControllerRotationRoll = true;
	bUseControllerRotationYaw = true;
	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	GetCharacterMovement()->bOrientRotationToMovement = false;
}

void AWOGBaseEnemy::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWOGBaseEnemy, OwnerAttacker);
	DOREPLIFETIME(AWOGBaseEnemy, OwnerSquad);
	DOREPLIFETIME(AWOGBaseEnemy, SquadUnitIndex);
	DOREPLIFETIME(AWOGBaseEnemy, CurrentEnemyState);
}

void AWOGBaseEnemy::BeginPlay()
{
	Super::BeginPlay();

	GiveDefaultAbilities();
	ApplyDefaultEffects();
}

void AWOGBaseEnemy::Destroyed()
{
	Super::Destroyed();
}

void AWOGBaseEnemy::OnStartAttack()
{
	HitActorsToIgnore.Empty();
}

void AWOGBaseEnemy::OnAttackHit(FHitResult Hit, UPrimitiveComponent* WeaponMesh)
{
	//Handle early returns
	if (!Hit.bBlockingHit || !Hit.GetActor()) return;
	if (HitActorsToIgnore.Contains(Hit.GetActor())) return;

	HitActorsToIgnore.AddUnique(Hit.GetActor());

	ProcessHit(Hit, WeaponMesh);
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
	if (AgressorCharacter && AgressorCharacter->HasMatchingGameplayTag(TAG_State_Weapon_AttackHeavy) && IsHitFrontal(60.f, this, FVector::Zero(), InstigatorWeapon))
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

void AWOGBaseEnemy::ProcessMagicHit(const FHitResult& Hit, const FMagicDataTable& MagicData)
{

}

void AWOGBaseEnemy::HandleStateElimmed(AController* InstigatedBy)
{
	Elim(false);
}

AWOGBaseSquad* AWOGBaseEnemy::GetEnemyOwnerSquad_Implementation()
{
	return OwnerSquad;
}

int32 AWOGBaseEnemy::GetEnemySquadUnitIndex_Implementation()
{
	return SquadUnitIndex;
}

void AWOGBaseEnemy::SetOwnerAttacker(AWOGAttacker* NewOwner)
{
	if (!HasAuthority()) return;
	if (!NewOwner)
	{
		UE_LOG(WOGLogSpawn, Error, TEXT("Invalid AttackerOwner on spawned Minion: %s"), *GetNameSafe(this));
		return;
	}

	OwnerAttacker = NewOwner;
}

void AWOGBaseEnemy::SetOwnerSquad(AWOGBaseSquad* NewOwnerSquad)
{
	if (!HasAuthority()) return;
	if (!NewOwnerSquad)
	{
		UE_LOG(WOGLogSpawn, Error, TEXT("Invalid OwnerSquad on spawned Minion: %s"), *GetNameSafe(this));
		return;
	}

	OwnerSquad = NewOwnerSquad;
}

void AWOGBaseEnemy::SetSquadUnitIndex(const int32& NewIndex)
{
	if (!HasAuthority()) return;

	SquadUnitIndex = NewIndex;
}

void AWOGBaseEnemy::SetCurrentEnemyState(const EEnemyState& NewState)
{
	if (!HasAuthority()) return;

	CurrentEnemyState = NewState;

	OnEnemyStateChangedDelegate.Broadcast();
	UE_LOG(WOGLogSpawn, Display, TEXT("EnemyState Changed"));
}

void AWOGBaseEnemy::Elim(bool bPlayerLeftGame)
{
	if (OwnerSquad)
	{
		OwnerSquad->DeregisterDeadSquadMember(this);
	}

	Multicast_Elim(bPlayerLeftGame);

	/*
	**Handle destroy timer
	*/
	GetWorld()->GetTimerManager().SetTimer(ElimTimer, this, &ThisClass::ElimTimerFinished, ElimDelay);
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
}

void AWOGBaseEnemy::ElimTimerFinished()
{
	Destroy();
}

