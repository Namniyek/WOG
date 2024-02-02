#pragma once

#include "CoreMinimal.h"
#include "GameplayTags.h"
#include "GameplayTagContainer.h"
#include "Data/WOGGameplayTags.h"
#include "Engine/DataTable.h"
#include "Data/WOGDataTypes.h"
#include "CharacterTypes.generated.h"

class UAnimMontage;
class USoundCue;

USTRUCT(BlueprintType)
struct FCharacterAbilityData
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GAS")
	TArray<TSubclassOf<class UWOGGameplayAbilityBase>> Abilities = { nullptr };

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GAS")
	TArray<TSubclassOf<class UGameplayEffect>> Effects = { nullptr };

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Inventory")
	TArray<TSubclassOf<class AWOGBaseWeapon>> Weapons = { nullptr };

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Inventory")
	TArray<TSubclassOf<class AWOGBaseMagic>> Magics = { nullptr };
};

USTRUCT(BlueprintType)
struct FCharacterData
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Base Data")
	bool bIsAttacker = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Base Data")
	bool bIsMale = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GAS")
	TSubclassOf<UGameplayEffect> SwitchAbilityCooldownEffect = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Vocals | Combat")
	TObjectPtr<USoundCue> AttackLightSound = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Vocals | Combat")
	TObjectPtr<USoundCue> AttackHeavySound = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Vocals | Combat")
	TObjectPtr<USoundCue> AttackLongSound = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Vocals | Combat")
	TObjectPtr<USoundCue> AttackBuildUpSound = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Vocals | Combat")
	TObjectPtr<USoundCue> BlockImpactSound = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Vocals | Combat")
	TObjectPtr<USoundCue> RangedThrowSound = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Vocals | Combat")
	TObjectPtr<USoundCue> MagicCastLongSound = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Vocals | Combat")
	TObjectPtr<USoundCue> MagicCastShortSound = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Vocals | Hurt")
	TObjectPtr<USoundCue> DeathSound = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Vocals | Hurt")
	TObjectPtr<USoundCue> PainShortSound = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Vocals | Hurt")
	TObjectPtr<USoundCue> PainLongSound = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Vocals | Hurt")
	TObjectPtr<USoundCue> MagicBurnSound = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Vocals | Hurt")
	TObjectPtr<USoundCue> MagicShockedSound = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Vocals | Hurt")
	TObjectPtr<USoundCue> MagicFrozenSound = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Vocals | Movement")
	TObjectPtr<USoundCue> TiredSound = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Vocals | Movement")
	TObjectPtr<USoundCue> EffortShortSound = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Vocals | Movement")
	TObjectPtr<USoundCue> EffortLongSound = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animations")
	UAnimMontage* ActionsMontage = nullptr;
};

USTRUCT(BlueprintType)
struct FPlayerCharacterEquipmentSnapshot
{
	GENERATED_USTRUCT_BODY();

	TPair<TSubclassOf<AActor>, int32> ConsumableReference;
};

/** Struct used to define the types of spawnables available */
USTRUCT(BlueprintType)
struct FSpawnables : public FTableRowBase
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "UI|Name"))
	FText Name = FText();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "UI|Icon"))
	TObjectPtr<UTexture2D> Icon = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Mesh", MakeStructureDefaultValue = "None"))
	TObjectPtr<UStaticMesh> Mesh = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Actor", MakeStructureDefaultValue = "None"))
	TObjectPtr<UClass> Actor = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Amount Units", MakeStructureDefaultValue = "1"))
	int32 AmountUnits = 0;

	/**Capsule half height*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "HeightOffset", MakeStructureDefaultValue = "1"))
	float HeightOffset = 0.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Initial Setup")
	FCharacterAbilityData DefaultAbilitiesAndEffects;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Initial Setup")
	EEnemySquadType SquadType = EEnemySquadType::EEST_Melee;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Combat")
	float BaseDamage = 10.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Combat")
	TObjectPtr<UAnimMontage> AttackMontage = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Combat")
	float AttackRange = 150.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Combat")
	float DefendRange = 400.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Combat")
	TSubclassOf<UGameplayEffect> DamageEffect = nullptr;
};