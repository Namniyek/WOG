#pragma once

#include "CoreMinimal.h"
#include "CharacterTypes.generated.h"

class USoundCue;

UENUM(BlueprintType)
enum class ECosmeticHit : uint8
{
	ECH_BodyHit UMETA(DisplayName = "Body Hit"),
	ECH_BlockingWeapon UMETA(DisplayName = "Blocking Weapon"),
	ECH_AttackingWeapon UMETA(DisplayName = "Attacking Weapon"),

	ECH_MAX UMETA(DisplayName = "DefaultMAX")
};

UENUM(BlueprintType)
enum class EAbilityType : uint8
{
	EAT_Projectile UMETA(DisplayName = "Projectile"),
	EAT_AOE UMETA(DisplayName = "Area Of Effect"),
	EAT_Instant UMETA(DisplayName = "Instant"),
	EAT_Buff UMETA(DisplayName = "Buff"),

	EAT_MAX UMETA(DisplayName = "DefaultMAX")
};

UENUM(BlueprintType)
enum class EAbilityInputType : uint8
{
	EAI_Instant UMETA(DisplayName = "Instant"),
	EAI_Hold UMETA(DisplayName = "Hold"),

	EAI_MAX UMETA(DisplayName = "DefaultMAX")
};

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
};

UENUM(BlueprintType)
enum class EWOGAbilityInputID : uint8
{
	//Base
	None			UMETA(DisplayName = "None"),
	Confirm			UMETA(DisplayName = "Confirm"),
	Cancel			UMETA(DisplayName = "Cancel"),

	//Input abilities
	Ability1		UMETA(DisplayName = "Ability1"),
	Ability2		UMETA(DisplayName = "Ability2"),
	Ability3		UMETA(DisplayName = "Ability3"),
	Ability4		UMETA(DisplayName = "Ability4"),

	//Weapons
	WeaponEquip		UMETA(DisplayName = "WeaponEquip"),
	WeaponUnequip	UMETA(DisplayName = "WeaponUnequip"),
	AttackLight		UMETA(DisplayName = "AttackLight"),
	AttackHeavy		UMETA(DisplayName = "AttackHeavy"),
	Block			UMETA(DisplayName = "Block"),
	Ranged			UMETA(DisplayName = "Ranged"),

	//Character
	Sprint			UMETA(DisplayName = "Sprint"),
	Dodge			UMETA(DisplayName = "Dodge"),
	Jump			UMETA(DisplayName = "Jump")
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

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Vocals")
	TObjectPtr<USoundCue> AttackLightSound = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Vocals")
	TObjectPtr<USoundCue> AttackHeavySound = nullptr;
};

