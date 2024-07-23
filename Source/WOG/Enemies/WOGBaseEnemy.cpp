// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGBaseEnemy.h"
#include "WOG.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "WOG/Weapons/WOGBaseWeapon.h"
#include "Kismet/GameplayStatics.h"
#include "ActorComponents/WOGAbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Data/WOGGameplayTags.h"
#include "Components/CapsuleComponent.h"
#include "PlayerCharacter/WOGAttacker.h"
#include "AI/Combat/WOGBaseSquad.h"
#include "Interfaces/BuildingInterface.h"
#include "Interfaces/AttributesInterface.h"
#include "AbilitySystem/AttributeSets/WOGAttributeSetBase.h"
#include "Magic/WOGBaseMagic.h"
#include "NiagaraFunctionLibrary.h"
#include "Sound/SoundCue.h"
#include "Libraries/WOGBlueprintLibrary.h"
#include "Magic/Projectile/WOGBaseMagicProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"

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
	MidAttackTagIndex = 0;
	CloseAttackTagIndex = 0;
	RangedAttackTagIndex = 0;

	CharacterData.bIsAttacker = true;

	GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPose;
	GetMesh()->SetGenerateOverlapEvents(true);
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
	DOREPLIFETIME(AWOGBaseEnemy, MidAttackTagIndex);
	DOREPLIFETIME(AWOGBaseEnemy, CloseAttackTagIndex);
	DOREPLIFETIME(AWOGBaseEnemy, RangedAttackTagIndex);
	DOREPLIFETIME(AWOGBaseEnemy, CosmeticsDataAsset);
}

float AWOGBaseEnemy::DoStuff_Implementation_Implementation(const float& NewParam) const
{
	return 0.f;
}

void AWOGBaseEnemy::BeginPlay()
{
	Super::BeginPlay();

	if (CharacterMI)
	{
		GetMesh()->SetMaterial(0, CharacterMI);
	}

	FTimerHandle CosmeticTimer;
	GetWorldTimerManager().SetTimer(CosmeticTimer, this, &ThisClass::HandleSpawnCosmetics, 0.1f);
	StartDissolve(true);

	if (AbilitySystemComponent && AttributeSet && HasAuthority())
	{
		ApplyDefaultEffects();
		GiveDefaultAbilities();
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

	MidAttackTagIndex = FMath::RandRange(0, AttackTagsMap.Num()-1);
	CloseAttackTagIndex = FMath::RandRange(0, CloseAttackTagsMap.Num() - 1);
	RangedAttackTagIndex = FMath::RandRange(0, RangedAttackTagsMap.Num() - 1);
}

void AWOGBaseEnemy::OnAttackHit(FHitResult Hit, UPrimitiveComponent* WeaponMesh)
{
	//Handle early returns
	if (!Hit.bBlockingHit || !Hit.GetActor()) return;
	if (HitActorsToIgnore.Contains(Hit.GetActor())) return;

	HitActorsToIgnore.AddUnique(Hit.GetActor());

	ProcessHit(Hit, WeaponMesh);
	UE_LOG(WOGLogCombat, Warning, TEXT("ProcessHit() called on %s"), *UEnum::GetValueAsString(GetLocalRole()));
}

void AWOGBaseEnemy::ProcessHit(FHitResult Hit, UPrimitiveComponent* WeaponMesh)
{
	if (!WeaponMesh)
	{
		UE_LOG(WOGLogCombat, Error, TEXT("Invalid WeaponMesh"));
		return;
	}

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
		UE_LOG(WOGLogCombat, Warning, TEXT("DamageToApply after DamageReduction of %f : %f"), DamageReduction, DamageToApply);

		AttributesInterface->Execute_BroadcastHit(Hit.GetActor(), this, Hit, DamageToApply, this);
	}
}

void AWOGBaseEnemy::ProcessRangedHit(const FHitResult& Hit, const float& DamageToApply, AActor* AggressorWeapon)
{
	if (!Hit.GetActor() || Hit.GetActor() == this)
	{
		UE_LOG(LogTemp, Error, TEXT("No Victim actor"));
		return;
	}
	
	//Get the Damage to apply values:
	float LocalDamageToApply = BaseDamage;
	if (HasMatchingGameplayTag(TAG_State_Weapon_AttackHeavy) && HasAuthority())
	{
		LocalDamageToApply = BaseDamage * 2;
	}

	//Check if we hit build and apply build damage
	IBuildingInterface* BuildInterface = Cast<IBuildingInterface>(Hit.GetActor());
	if (BuildInterface && HasAuthority())
	{
		BuildInterface->Execute_DealDamage(Hit.GetActor(), LocalDamageToApply, this);
		UE_LOG(LogTemp, Warning, TEXT("Build damaged with %f"), LocalDamageToApply);
		return;
	}

	//Check if we hit other character
	IAttributesInterface* AttributesInterface = Cast<IAttributesInterface>(Hit.GetActor());
	if (AttributesInterface)
	{
		bool FoundAttribute;
		float DamageReduction = UAbilitySystemBlueprintLibrary::GetFloatAttribute(Hit.GetActor(), AttributeSet->GetDamageReductionAttribute(), FoundAttribute);
		LocalDamageToApply *= (1 - DamageReduction);
		UE_LOG(LogTemp, Warning, TEXT("DamageToApply after DamageReduction of %f : %f"), DamageReduction, LocalDamageToApply);

		AttributesInterface->Execute_BroadcastHit(Hit.GetActor(), this, Hit, LocalDamageToApply, AggressorWeapon);
	}
}

void AWOGBaseEnemy::ProcessMagicHit(const FHitResult& Hit, const FMagicDataTable& MagicData)
{
	if (!Hit.GetActor())
	{
		UE_LOG(WOGLogSpawn, Error, TEXT("No Victim actor for WOGBaseEnemy ProcessMagicHit()"));
		return;
	}

	//Get the Damage to apply values:
	float DamageToApply = 0.f;;
	if (HasAuthority())
	{
		DamageToApply = MagicData.Value * MagicData.ValueMultiplier;
	}

	//Check if we hit build and apply build damage
	IBuildingInterface* BuildInterface = Cast<IBuildingInterface>(Hit.GetActor());
	if (BuildInterface && HasAuthority())
	{
		BuildInterface->Execute_DealDamage(Hit.GetActor(), DamageToApply, this);
		UE_LOG(LogTemp, Warning, TEXT("Build damaged with %f"), DamageToApply);
		return;
	}

	//Check if we hit other character
	IAttributesInterface* AttributesInterface = Cast<IAttributesInterface>(Hit.GetActor());
	if (AttributesInterface)
	{
		AttributesInterface->Execute_BroadcastMagicHit(Hit.GetActor(), this, Hit, MagicData);
	}
}

void AWOGBaseEnemy::BroadcastHit_Implementation(AActor* AggressorActor, const FHitResult& Hit, const float& DamageToApply, AActor* InstigatorWeapon)
{
	if (HasMatchingGameplayTag(TAG_State_Dead)) return;
	if (HasMatchingGameplayTag(TAG_State_Minion_Dodging)) return;
	if (!AbilitySystemComponent.Get()) return;

	AWOGBaseCharacter* AggressorCharacter = Cast<AWOGBaseCharacter>(AggressorActor);
	AWOGBaseWeapon* AggressorWeapon = Cast<AWOGBaseWeapon>(InstigatorWeapon);

	if (!AggressorCharacter)
	{
		UE_LOG(WOGLogCombat, Error, TEXT("No valid Agressor Character"));
		return;
	}
	if (!AggressorWeapon)
	{
		UE_LOG(WOGLogCombat, Warning, TEXT("No valid Agressor Weapon"));
		return;
	}

	//Store the last hit result
	LastHitResult = Hit;

	//Handle Ranged Weapon Throw Hit
	if (AggressorCharacter->HasMatchingGameplayTag(TAG_State_Weapon_Ranged_Throw) || AggressorCharacter->HasMatchingGameplayTag(TAG_State_Weapon_Ranged_AOE))
	{
		//Victim hit by shield throw
		if (AggressorWeapon && AggressorWeapon->GetWeaponData().WeaponTag.MatchesTag(TAG_Inventory_Weapon_Shield))
		{
			AbilityActivationPayload = FGameplayEventData();
			AbilityActivationPayload.EventTag = AggressorWeapon->GetWeaponData().RangedTag;
			AbilityActivationPayload.EventMagnitude = AggressorWeapon->GetWeaponData().StunDuration;

			UWOGBlueprintLibrary::TryActivateAbilityByTagWithData(
				this,
				AggressorWeapon->GetWeaponData().RangedTag,
				AbilityActivationPayload);

			FGameplayCueParameters CueParams;
			CueParams.Location = Hit.ImpactPoint;
			CueParams.EffectCauser = AggressorCharacter;
			AbilitySystemComponent->ExecuteGameplayCueLocal(TAG_Cue_Weapon_BodyHit, CueParams);
		}

		//Victim hit by dual weapon throw
		if (AggressorWeapon && AggressorWeapon->GetWeaponData().WeaponTag.MatchesTag(TAG_Inventory_Weapon_DualWield))
		{
			AbilityActivationPayload = FGameplayEventData();
			AbilityActivationPayload.EventTag = AggressorWeapon->GetWeaponData().RangedTag;
			AbilityActivationPayload.EventMagnitude = AggressorWeapon->GetWeaponData().StunDuration;

			UWOGBlueprintLibrary::TryActivateAbilityByTagWithData(
				this,
				AggressorWeapon->GetWeaponData().RangedTag,
				AbilityActivationPayload);

			FGameplayCueParameters CueParams;
			CueParams.Location = Hit.ImpactPoint;
			CueParams.EffectCauser = AggressorCharacter;
			AbilitySystemComponent->ExecuteGameplayCueLocal(TAG_Cue_Weapon_BodyHit, CueParams);
		}

		//Victim hit by two-handed ranged attack
		if (AggressorWeapon && AggressorWeapon->GetWeaponData().WeaponTag.MatchesTag(TAG_Inventory_Weapon_TwoHanded))
		{
			AbilityActivationPayload = FGameplayEventData();
			AbilityActivationPayload.EventTag = AggressorWeapon->GetWeaponData().RangedTag;

			UWOGBlueprintLibrary::TryActivateAbilityByTagWithData(
				this,
				AggressorWeapon->GetWeaponData().RangedTag,
				AbilityActivationPayload);
		}
	}

	//Handle blocked hits for victim and aggressor
	if (HasMatchingGameplayTag(TAG_State_Minion_Blocking) && IsHitFrontal(60.f, this, FVector::Zero(), AggressorActor))
	{
		if (AggressorCharacter->HasMatchingGameplayTag(TAG_State_Weapon_AttackHeavy))
		{
			//Attacker used heavy attack on victim while guarding:
			//Handle stun on victim
			AbilityActivationPayload = FGameplayEventData();
			AbilityActivationPayload.EventTag = TAG_Ability_Debuff_Stun;
			AbilityActivationPayload.EventMagnitude = 5.f;

			UWOGBlueprintLibrary::TryActivateAbilityByTagWithData(
				this,
				TAG_Ability_Debuff_Stun,
				AbilityActivationPayload);
			return;
		}

		//Attacker didn't use heavy attack on victim while guarding:
		//Handle stagger on victims
		AbilityActivationPayload = FGameplayEventData();
		AbilityActivationPayload.EventTag = TAG_Ability_Debuff_Stagger;

		UWOGBlueprintLibrary::TryActivateAbilityByTagWithData(
			this,
			TAG_Ability_Debuff_Stagger,
			AbilityActivationPayload);
		return;
	}

	//Apply HitReact or KO to character
	if (!HasMatchingGameplayTag(TAG_State_Minion_Blocking) && AggressorCharacter->HasMatchingGameplayTag(TAG_State_Weapon_AttackHeavy) && IsHitFrontal(60.f, this, FVector::Zero(), InstigatorWeapon))
	{
		//Send KO ability to victim
		AbilityActivationPayload = FGameplayEventData();
		AbilityActivationPayload.EventTag = TAG_Event_Debuff_KO;

		UWOGBlueprintLibrary::TryActivateAbilityByTagWithData(
			this,
			TAG_Ability_Debuff_KO,
			AbilityActivationPayload);
	}
	else
	{
		//Send light HitReact ability to victim
		AbilityActivationPayload = FGameplayEventData();
		AbilityActivationPayload.EventTag = TAG_Ability_Debuff_HitReact;
		AbilityActivationPayload.Instigator = InstigatorWeapon;
		AbilityActivationPayload.EventMagnitude = AggressorWeapon->GetWeaponData().StunDuration;

		UWOGBlueprintLibrary::TryActivateAbilityByTagWithData(
			this,
			TAG_Ability_Debuff_HitReact,
			AbilityActivationPayload);

		FGameplayCueParameters CueParams;
		CueParams.Location = Hit.ImpactPoint;
		CueParams.EffectCauser = AggressorCharacter;
		AbilitySystemComponent->ExecuteGameplayCueLocal(TAG_Cue_Weapon_BodyHit, CueParams);

	}

	//Apply damage to character if authority
	if (HasAuthority() && AggressorCharacter && AggressorWeapon && AggressorWeapon->GetWeaponData().WeaponDamageEffect)
	{
		FGameplayEffectContextHandle DamageContext = AbilitySystemComponent.Get()->MakeEffectContext();
		DamageContext.AddInstigator(AggressorCharacter, AggressorWeapon);
		DamageContext.AddHitResult(Hit);

		FGameplayEffectSpecHandle OutSpec = AbilitySystemComponent->MakeOutgoingSpec(AggressorWeapon->GetWeaponData().WeaponDamageEffect, 1, DamageContext);
		UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(OutSpec, FGameplayTag::RequestGameplayTag(TEXT("Damage.Attribute.Health")), -DamageToApply);
		auto ActiveGEHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*OutSpec.Data);
		if (ActiveGEHandle.WasSuccessfullyApplied())
		{
			UE_LOG(WOGLogCombat, Display, TEXT("Damage applied to %s : %f"), *GetNameSafe(this), DamageToApply);
		}
		else
		{
			UE_LOG(WOGLogCombat, Error, TEXT("Damage to %s not applied"), *GetNameSafe(this));
		}
	}
}

void AWOGBaseEnemy::BroadcastMagicHit_Implementation(AActor* AggressorActor, const FHitResult& Hit,
	const FMagicDataTable& AggressorMagicData)
{
		//Handle early returns
	if (HasMatchingGameplayTag(TAG_State_Dead)) return;
	if (HasMatchingGameplayTag(TAG_State_Minion_Dodging)) return;
	if (!AbilitySystemComponent.Get()) return;

	if (!AggressorActor) return;
	TObjectPtr<AWOGBaseCharacter> AggressorCharacter = Cast<AWOGBaseCharacter>(AggressorActor);

	//Handle more early returns and warnings
	if (!AggressorCharacter)
	{
		UE_LOG(WOGLogCombat, Error, TEXT("No valid Agressor Character"));
		return;
	}

	//Store the last hit result and calculate damage
	LastHitResult = Hit;
	float LocalDamageToApply = AggressorMagicData.Value * AggressorMagicData.ValueMultiplier;

	bool FoundAttribute;
	const float DamageReduction = UAbilitySystemBlueprintLibrary::GetFloatAttribute(Hit.GetActor(), AttributeSet->GetDamageReductionAttribute(), FoundAttribute);
	LocalDamageToApply *= (1 - DamageReduction);
	UE_LOG(WOGLogCombat, Warning, TEXT("DamageToApply after DamageReduction of %f : %f"), DamageReduction, LocalDamageToApply);

	//Apply secondary effect
	if (UKismetSystemLibrary::IsValidClass(AggressorMagicData.SecondaryEffect))
	{
		FGameplayEffectContextHandle SecondaryContext = AbilitySystemComponent.Get()->MakeEffectContext();
		SecondaryContext.AddInstigator(AggressorCharacter, AggressorCharacter);
		// ReSharper disable once CppExpressionWithoutSideEffects
		ApplyGameplayEffectToSelf(AggressorMagicData.SecondaryEffect, SecondaryContext, AggressorMagicData.SecondaryEffectDuration);
	}

	//Handle AOE KO
	if (AggressorMagicData.AbilityType == EAbilityType::EAT_AOE)
	{
		//Send KO ability to victim
		AbilityActivationPayload = FGameplayEventData();
		AbilityActivationPayload.EventTag = TAG_Event_Debuff_KO;

		UWOGBlueprintLibrary::TryActivateAbilityByTagWithData(
			this,
			TAG_Ability_Debuff_KO,
			AbilityActivationPayload);
	}

	//Handle blocked hits for victim and aggressor
	if (HasMatchingGameplayTag(TAG_State_Minion_Blocking) && IsHitFrontal(60.f, this, Hit.ImpactPoint, nullptr))
	{
		//Condition to sort out different types of magic
		if (AggressorMagicData.AbilityType == EAbilityType::EAT_Projectile)
		{
			LocalDamageToApply = 0.f;
		}
	}

	//Handle unguarded hit to victim
	if (!HasMatchingGameplayTag(TAG_State_Minion_Blocking))
	{
		//Condition to sort out different types of magic
		if (AggressorMagicData.AbilityType == EAbilityType::EAT_Projectile)
		{
			//Send light HitReact ability to victim
			AbilityActivationPayload = FGameplayEventData();
			AbilityActivationPayload.EventTag = TAG_Ability_Debuff_HitReact;
			AbilityActivationPayload.Instigator = AggressorCharacter;
			AbilityActivationPayload.EventMagnitude = 1.f;

			UWOGBlueprintLibrary::TryActivateAbilityByTagWithData(
				this,
				TAG_Ability_Debuff_HitReact,
				AbilityActivationPayload);

			FGameplayCueParameters CueParams;
			CueParams.Location = Hit.ImpactPoint;
			CueParams.EffectCauser = AggressorCharacter;
			AbilitySystemComponent->ExecuteGameplayCueLocal(TAG_Cue_Weapon_BodyHit, CueParams);
		}
	}

	//Apply damage to victim if authority
	if (HasAuthority() && AggressorCharacter && AbilitySystemComponent.Get() && AggressorMagicData.DamageEffect)
	{
		FGameplayEffectContextHandle DamageContext = AbilitySystemComponent.Get()->MakeEffectContext();
		DamageContext.AddInstigator(AggressorCharacter, AggressorCharacter);
		DamageContext.AddHitResult(Hit);

		FGameplayEffectSpecHandle OutSpec = AbilitySystemComponent->MakeOutgoingSpec(AggressorMagicData.DamageEffect, 1, DamageContext);
		UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(OutSpec, FGameplayTag::RequestGameplayTag(TEXT("Damage.Attribute.Health")), -LocalDamageToApply);
		AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*OutSpec.Data);
		UE_LOG(WOGLogCombat, Error, TEXT("Damage applied to %s : %f"), *GetNameSafe(this), LocalDamageToApply);
	}
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

AActor* AWOGBaseEnemy::GetSquadCurrentTargetActor_Implementation()
{
	if(!OwnerSquad) return nullptr;
	return OwnerSquad->GetCurrentTargetActor();
}

float AWOGBaseEnemy::GetAttackRangeValue_Implementation()
{
	return AttackRange;
}

float AWOGBaseEnemy::GetDefendRangeValue_Implementation()
{
	return DefendRange;
}

void AWOGBaseEnemy::HandleSpawnCosmetics() const
{
	if (CosmeticsDataAsset && CosmeticsDataAsset->SpawnSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, CosmeticsDataAsset->SpawnSound, GetActorLocation());
	}
	if (CosmeticsDataAsset && CosmeticsDataAsset->SpawnParticleSystem)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, CosmeticsDataAsset->SpawnParticleSystem, GetActorLocation(), GetActorRotation());
	}
}

void AWOGBaseEnemy::HandleDestroyCosmetics() const
{
	if (CosmeticsDataAsset && CosmeticsDataAsset->DestroySound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, CosmeticsDataAsset->DestroySound, GetActorLocation());
	}
	if (CosmeticsDataAsset && CosmeticsDataAsset->DestroyParticleSystem)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, CosmeticsDataAsset->DestroyParticleSystem, GetActorLocation(), GetActorRotation());
	}
}

void AWOGBaseEnemy::ExecuteGameplayCueWithCosmeticsDataAsset(const FGameplayTag& CueTag)
{
	if (!HasAuthority())
	{
		return;
	}

	if (AbilitySystemComponent)
	{
	FGameplayCueParameters CueParams;
	CueParams.SourceObject = this;
	AbilitySystemComponent->ExecuteGameplayCue(CueTag, CueParams);
	}
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

int32 AWOGBaseEnemy::GetAttackIndex_Implementation() const
{
	return MidAttackTagIndex;
}

FGameplayTag AWOGBaseEnemy::GetAttackData_Implementation(int32& TokensNeeded)
{
	TokensNeeded = 100;
	if (AttackTagsMap.IsEmpty()) return FGameplayTag();

	TArray<TPair<FGameplayTag, int32>> Array = AttackTagsMap.Array();
	if(Array.IsEmpty()) return FGameplayTag();

	for (int32 i = 0; i < Array.Num(); i++)
	{
		if (i == MidAttackTagIndex)
		{
			TokensNeeded = Array[i].Value;
			return Array[i].Key;
		}
	}

	return FGameplayTag();
}

FGameplayTag AWOGBaseEnemy::GetRangedAttackData_Implementation(int32& TokensNeeded)
{
	TokensNeeded = 100;
	if (RangedAttackTagsMap.IsEmpty()) return FGameplayTag();

	TArray<TPair<FGameplayTag, int32>> Array = RangedAttackTagsMap.Array();
	if (Array.IsEmpty()) return FGameplayTag();

	for (int32 i = 0; i < Array.Num(); i++)
	{
		if (i == RangedAttackTagIndex)
		{
			TokensNeeded = Array[i].Value;
			return Array[i].Key;
		}
	}

	return FGameplayTag();
}

FGameplayTag AWOGBaseEnemy::GetCloseAttackData_Implementation(int32& TokensNeeded)
{
	TokensNeeded = 100;
	if (CloseAttackTagsMap.IsEmpty()) return FGameplayTag();

	TArray<TPair<FGameplayTag, int32>> Array = CloseAttackTagsMap.Array();
	if (Array.IsEmpty()) return FGameplayTag();

	for (int32 i = 0; i < Array.Num(); i++)
	{
		if (i == CloseAttackTagIndex)
		{
			TokensNeeded = Array[i].Value;
			return Array[i].Key;
		}
	}

	return FGameplayTag();
}

FGameplayTag AWOGBaseEnemy::GetAttackDataAtIndex_Implementation(const int32& Index, int32& TokensNeeded)
{
	TokensNeeded = 100;
	if (AttackTagsMap.IsEmpty()) return FGameplayTag();

	TArray<TPair<FGameplayTag, int32>> Array = AttackTagsMap.Array();
	if (Array.IsEmpty()) return FGameplayTag();

	TokensNeeded = Array[Index].Value;
	return Array[Index].Key;
}

void AWOGBaseEnemy::DefineAttackTagIndex_Implementation()
{
	DefineNextAttackTagIndex();
}

void AWOGBaseEnemy::IncreaseComboIndex_Implementation()
{
	if (!HasAuthority()) return;
	
	ComboIndex++;
	bComboWindowOpen = true; 
}

void AWOGBaseEnemy::ResetComboIndex_Implementation()
{
	if (!HasAuthority()) return;

	ComboIndex = 1;
	bComboWindowOpen = false;
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

void AWOGBaseEnemy::SetSecondaryDamageEffect(const TSubclassOf<UGameplayEffect>& NewDamageEffect)
{
	if (!HasAuthority()) return;
	SecondaryDamageEffect = NewDamageEffect;
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
	ExecuteGameplayCueWithCosmeticsDataAsset(TAG_Cue_Minion_Destroy);
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

void AWOGBaseEnemy::Multicast_SpawnProjectile_Implementation(const FMagicDataTable& MagicData, const FName& SpawnSocket, const FRotator& SpawnRotation, bool bIsHoming)
{
	SpawnProjectile(MagicData, SpawnSocket, SpawnRotation, bIsHoming);
}

void AWOGBaseEnemy::SpawnProjectile(const FMagicDataTable& MagicData, const FName& SpawnSocket, const FRotator& SpawnRotation, bool bIsHoming)
{
	if (!IsValid(MagicData.ProjectileClass)) return;

	FTransform SpawnTransform;
	SpawnTransform.SetLocation(GetMesh()->GetSocketLocation(SpawnSocket));

	AActor* CurrentTarget = ISpawnInterface::Execute_GetSquadCurrentTargetActor(this);
	SpawnTransform.SetRotation(SpawnRotation.Quaternion());
	SpawnTransform.SetScale3D(FVector(1.f));


	TObjectPtr<AWOGBaseMagicProjectile> Projectile = GetWorld()->SpawnActorDeferred<AWOGBaseMagicProjectile>(
		MagicData.ProjectileClass,
		SpawnTransform,
		this,
		this,
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);

	if (Projectile)
	{
		Projectile->SetMagicData(MagicData);
		
		if (bIsHoming && CurrentTarget)
		{
			Projectile->Target = CurrentTarget;
			Projectile->GetProjectileMovementComponent()->HomingTargetComponent = CurrentTarget->GetRootComponent();
		}

		Projectile->GetProjectileMovementComponent()->bIsHomingProjectile = bIsHoming;
		Projectile->FinishSpawning(SpawnTransform);
	}
}

