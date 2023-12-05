#pragma once

#include "CoreMinimal.h"
#include "GameplayTags.h"
#include "GameplayTagContainer.h"
#include "Types/WOGGameplayTags.h"
#include "CharacterTypes.generated.h"

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
};

USTRUCT(BlueprintType)
struct FPlayerCharacterEquipmentSnapshot
{
	GENERATED_USTRUCT_BODY();

	TPair<TSubclassOf<AActor>, int32> ConsumableReference;
};