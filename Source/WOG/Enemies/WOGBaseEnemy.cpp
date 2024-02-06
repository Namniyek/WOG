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
#include "Interfaces/BuildingInterface.h"
#include "Interfaces/AttributesInterface.h"
#include "AbilitySystem/AttributeSets/WOGAttributeSetBase.h"

AWOGBaseEnemy::AWOGBaseEnemy()
{
	PrimaryActorTick.bCanEverTick = true;
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

	BaseDamage = 10.f;
	DamageEffect = nullptr;

	ComboIndex = 1;
	AttackTagIndex = 0;
}

void AWOGBaseEnemy::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (Material)
	{
		CharacterMI = UMaterialInstanceDynamic::Create(Material, this);
	}
}

void AWOGBaseEnemy::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWOGBaseEnemy, OwnerAttacker);
	DOREPLIFETIME(AWOGBaseEnemy, OwnerSquad);
	DOREPLIFETIME(AWOGBaseEnemy, SquadUnitIndex);
	DOREPLIFETIME(AWOGBaseEnemy, CurrentEnemyState);
	DOREPLIFETIME(AWOGBaseEnemy, AttackRange);
	DOREPLIFETIME(AWOGBaseEnemy, DefendRange);
	DOREPLIFETIME(AWOGBaseEnemy, ComboIndex);
	DOREPLIFETIME(AWOGBaseEnemy, AttackTagIndex);
}

void AWOGBaseEnemy::BeginPlay()
{
	Super::BeginPlay();

	GiveDefaultAbilities();
	ApplyDefaultEffects();

	if (CharacterMI)
	{
		GetMesh()->SetMaterial(0, CharacterMI);
	}
}

void AWOGBaseEnemy::Destroyed()
{
	Super::Destroyed();
}

void AWOGBaseEnemy::DefineNextComboIndex()
{
	if (!HasAuthority()) return;

	ComboIndex = FMath::RandRange(1, 3);
}

void AWOGBaseEnemy::OnStartAttack()
{
	HitActorsToIgnore.Empty();
}

void AWOGBaseEnemy::DefineNextAttackTagIndex()
{
	if (!HasAuthority()) return;

	AttackTagIndex = FMath::RandRange(0, AttackTagsContainer.Num()-1);
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
	if (!WeaponMesh) return;

	//Get the Damage to apply values:
	float DamageToApply = BaseDamage;
	if (HasMatchingGameplayTag(TAG_State_Weapon_AttackHeavy) && HasAuthority())
	{
		DamageToApply = BaseDamage * 2;
	}

	//Check if we hit build and apply build damage
	IBuildingInterface* BuildInterface = Cast<IBuildingInterface>(Hit.GetActor());
	if (BuildInterface && HasAuthority())
	{
		BuildInterface->Execute_DealDamage(Hit.GetActor(), DamageToApply, this);
		UE_LOG(WOGLogCombat, Warning, TEXT("Build damaged with %f"), DamageToApply);
		return;
	}

	//Check if we hit other character
	IAttributesInterface* AttributesInterface = Cast<IAttributesInterface>(Hit.GetActor());
	if (AttributesInterface)
	{
		bool FoundAttribute;
		float DamageReduction = UAbilitySystemBlueprintLibrary::GetFloatAttribute(Hit.GetActor(), AttributeSet->GetDamageReductionAttribute(), FoundAttribute);
		DamageToApply *= (1 - DamageReduction);
		UE_LOG(LogTemp, Warning, TEXT("DamageToApply after DamageReduction of %f : %f"), DamageReduction, DamageToApply);

		AttributesInterface->Execute_BroadcastHit(Hit.GetActor(), this, Hit, DamageToApply, this);
	}
}

void AWOGBaseEnemy::BroadcastHit_Implementation(AActor* AgressorActor, const FHitResult& Hit, const float& DamageToApply, AActor* InstigatorWeapon)
{
	if (HasMatchingGameplayTag(TAG_State_Dead)) return;
	if (HasMatchingGameplayTag(TAG_State_Dodging)) return;

	AWOGBaseCharacter* AgressorCharacter = Cast<AWOGBaseCharacter>(AgressorActor);
	AWOGBaseWeapon* AgressorWeapon = Cast<AWOGBaseWeapon>(InstigatorWeapon);

	if (!AgressorCharacter)
	{
		UE_LOG(WOGLogCombat, Error, TEXT("No valid Agressor Character"));
		return;
	}
	if (!AgressorWeapon)
	{
		UE_LOG(WOGLogCombat, Warning, TEXT("No valid Agressor Weapon"));
	}

	//Store the last hit result
	LastHitResult = Hit;

	//Handle Ranged Weapon Throw Hit
	if (AgressorCharacter->HasMatchingGameplayTag(TAG_State_Weapon_Ranged_Throw) || AgressorCharacter->HasMatchingGameplayTag(TAG_State_Weapon_Ranged_AOE))
	{
		//Victim hit by shield throw
		if (AgressorWeapon && AgressorWeapon->GetWeaponData().WeaponTag.MatchesTag(TAG_Inventory_Weapon_Shield))
		{
			FGameplayEventData EventPayload;
			EventPayload.EventTag = AgressorWeapon->GetWeaponData().RangedTag;
			EventPayload.EventMagnitude = AgressorWeapon->GetWeaponData().StunDuration;
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, AgressorWeapon->GetWeaponData().RangedTag, EventPayload);
			UE_LOG(WOGLogCombat, Warning, TEXT("Shield throw hit and applied: %s during %f seconds"), *AgressorWeapon->GetWeaponData().RangedTag.ToString(), AgressorWeapon->GetWeaponData().StunDuration);

			FGameplayCueParameters CueParams;
			CueParams.Location = Hit.ImpactPoint;
			CueParams.EffectCauser = AgressorCharacter;
			AbilitySystemComponent->ExecuteGameplayCueLocal(TAG_Cue_Weapon_BodyHit, CueParams);
		}

		//Victim hit by dual weapon throw
		if (AgressorWeapon && AgressorWeapon->GetWeaponData().WeaponTag.MatchesTag(TAG_Inventory_Weapon_DualWield))
		{
			FGameplayEventData EventPayload;
			EventPayload.EventTag = AgressorWeapon->GetWeaponData().RangedTag;
			EventPayload.EventMagnitude = AgressorWeapon->GetWeaponData().StunDuration;
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, AgressorWeapon->GetWeaponData().RangedTag, EventPayload);
			UE_LOG(WOGLogCombat, Warning, TEXT("Weapon throw hit and applied: %s during %f seconds"), *AgressorWeapon->GetWeaponData().RangedTag.ToString(), AgressorWeapon->GetWeaponData().StunDuration);

			FGameplayCueParameters CueParams;
			CueParams.Location = Hit.ImpactPoint;
			CueParams.EffectCauser = AgressorCharacter;
			AbilitySystemComponent->ExecuteGameplayCueLocal(TAG_Cue_Weapon_BodyHit, CueParams);
		}

		//Victim hit by two handed ranged attack
		if (AgressorWeapon && AgressorWeapon->GetWeaponData().WeaponTag.MatchesTag(TAG_Inventory_Weapon_TwoHanded))
		{
			FGameplayEventData EventPayload;
			EventPayload.EventTag = AgressorWeapon->GetWeaponData().RangedTag;
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, AgressorWeapon->GetWeaponData().RangedTag, EventPayload);
			UE_LOG(WOGLogCombat, Warning, TEXT("Weapon AOE hit and applied: %s"), *AgressorWeapon->GetWeaponData().RangedTag.ToString());
		}
	}

	//Apply HitReact or KO to character
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
	if (HasAuthority() && AgressorCharacter && AbilitySystemComponent.Get() && AgressorWeapon && AgressorWeapon->GetWeaponData().WeaponDamageEffect)
	{
		FGameplayEffectContextHandle DamageContext = AbilitySystemComponent.Get()->MakeEffectContext();
		DamageContext.AddInstigator(AgressorCharacter, AgressorWeapon);
		DamageContext.AddHitResult(Hit);

		FGameplayEffectSpecHandle OutSpec = AbilitySystemComponent->MakeOutgoingSpec(AgressorWeapon->GetWeaponData().WeaponDamageEffect, 1, DamageContext);
		UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(OutSpec, FGameplayTag::RequestGameplayTag(TEXT("Damage.Attribute.Health")), -DamageToApply);
		AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*OutSpec.Data);
		UE_LOG(WOGLogCombat, Display, TEXT("Damage applied to %s : %f"), *GetNameSafe(this), DamageToApply);
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

float AWOGBaseEnemy::GetAttackRangeValue_Implementation()
{
	return AttackRange;
}

float AWOGBaseEnemy::GetDefendRangeValue_Implementation()
{
	return DefendRange;
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

int32 AWOGBaseEnemy::GetComboIndex_Implementation()
{
	return ComboIndex;
}

FGameplayTag AWOGBaseEnemy::GetAttackTag_Implementation()
{
	if(AttackTagsContainer.IsValidIndex(AttackTagIndex))
	{
		return AttackTagsContainer.GetByIndex(AttackTagIndex);
	}
	
	return FGameplayTag();
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

void AWOGBaseEnemy::DefineComboIndex_Implementation()
{
	DefineNextComboIndex();
}

void AWOGBaseEnemy::DefineAttackTagIndex_Implementation()
{
	DefineNextAttackTagIndex();
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

void AWOGBaseEnemy::SetBaseDamage(const float& NewDamage)
{
	if (!HasAuthority()) return;
	BaseDamage = NewDamage;
}

void AWOGBaseEnemy::SetAttackMontage(UAnimMontage* NewMontage)
{
	if (!HasAuthority() || !NewMontage) return;
	AttackMontage = NewMontage;
}

void AWOGBaseEnemy::SetAttackRange(const float& NewRadius)
{
	if (!HasAuthority()) return;
	AttackRange = NewRadius;
}

void AWOGBaseEnemy::SetDefendRange(const float& NewRadius)
{
	if (!HasAuthority()) return;
	DefendRange = NewRadius;
}

void AWOGBaseEnemy::SetDamageEffect(const TSubclassOf<UGameplayEffect>& NewDamageEffect)
{
	if (!HasAuthority()) return;
	DamageEffect = NewDamageEffect;
}

void AWOGBaseEnemy::Elim(bool bPlayerLeftGame)
{
	if (OwnerSquad)
	{
		if (OwnerSquad->GetCurrentTargetActor() && OwnerSquad->GetCurrentTargetActor()->GetClass()->ImplementsInterface(UAttributesInterface::StaticClass()))
		{
			IAttributesInterface::Execute_RestoreAttackTokens(OwnerSquad->GetCurrentTargetActor(), 1);
		}

		OwnerSquad->DeregisterDeadSquadMember(this);
	}

	OnCharacterElimEvent();

	Multicast_StartDissolve();

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->DisableMovement();
		GetCharacterMovement()->StopMovementImmediately();
	}

	if (GetCapsuleComponent())
	{
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	/*
	**Handle destroy timer
	*/
	GetWorld()->GetTimerManager().SetTimer(ElimTimer, this, &ThisClass::ElimTimerFinished, ElimDelay);
}

void AWOGBaseEnemy::Multicast_Elim_Implementation(bool bPlayerLeftGame)
{
	GetMesh()->SetCollisionProfileName(FName("Ragdoll"));
	GetMesh()->bPauseAnims = true;
	if (GetCapsuleComponent())
	{
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void AWOGBaseEnemy::ElimTimerFinished()
{
	Multicast_Elim(false);
	Destroy();
}

