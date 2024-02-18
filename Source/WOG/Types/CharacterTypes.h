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
class AWOGBaseEnemy;

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

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Inventory")
	TArray<TSubclassOf<AActor>> DefaultItems = { nullptr };
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

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "1 - Base")
	EEnemySquadType SquadType = EEnemySquadType::EEST_Melee;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Spawn Name"), Category = "1 - Base")
	FText Name = FText();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Spawn Icon"), Category = "1 - Base")
	TObjectPtr<UTexture2D> Icon = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Spawn cost amount"), Category = "1 - Base")
	int32 CostAmount = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Spawn cost tag"), Category = "1 - Base")
	FGameplayTag CostTag = FGameplayTag();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Preview Mesh"), Category = "1 - Base")
	TObjectPtr<UStaticMesh> Mesh = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Minion Array"), Category = "1 - Base")
	TArray<TSubclassOf<AWOGBaseEnemy>> MinionArray = {};

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Amount Units"), Category = "1 - Base")
	int32 AmountUnits = 0;

	/**Capsule half height*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Height Offset"), Category = "1 - Base")
	float HeightOffset = 0.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "2 - Combat Setup")
	FCharacterAbilityData DefaultAbilitiesAndEffects;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "2 - Combat Setup")
	float BaseDamage = 10.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "2 - Combat Setup")
	TObjectPtr<UAnimMontage> AttackMontage = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "2 - Combat Setup")
	float AttackRange = 150.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "2 - Combat Setup")
	float DefendRange = 400.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "2 - Combat Setup")
	TSubclassOf<UGameplayEffect> DamageEffect = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Spawn Item Tag"), Category = "3 - Inventory")
	FGameplayTag ItemTag = FGameplayTag();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "4 - User Interface")
	FVendorItemData VendorItemData = FVendorItemData();

};