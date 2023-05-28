#pragma once

#include "CoreMinimal.h"
#include "CharacterTypes.generated.h"



UENUM(BlueprintType)
enum class ECharacterState : uint8
{
	ECS_Unnoccupied UMETA(DisplayName = "Unnoccupied"),
	ECS_Attacking UMETA(DisplayName = "Attacking"),
	ECS_Staggered UMETA(DisplayName = "Staggered"),

	ECS_MAX UMETA(DisplayName = "DefaultMAX")
};

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_Relax UMETA(DisplayName = "Relax"),
	EWT_SwordAndShield UMETA(DisplayName = "Sword and Shield"),
	EWT_AxeAndShield UMETA(DisplayName = "Axe and Shield"),
	EWT_HammerAndShield UMETA(DisplayName = "Hammer and Shield"),
	EWT_TwoHandedSword UMETA(DisplayName = "Two Handed Sword"),
	EWT_TwoHandedAxe UMETA(DisplayName = "Two Handed Axe"),
	EWT_TwoHandedHammer UMETA(DisplayName = "Two Handed Hammer"),
	EWT_DualWieldSword UMETA(DisplayName = "Dual Wield Sword"),
	EWT_DualWieldAxe UMETA(DisplayName = "Dual Wield Axe"),
	EWT_DualWieldHammer UMETA(DisplayName = "Dual Wield Hammer"),
	EWT_BowAndArrow UMETA(DisplayName = "Bow and Arrow"),

	EWT_MAX UMETA(DisplayName = "DefaultMAX")
};

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
	EAT_Relax UMETA(DisplayName = "Relax"),
	EAT_Projectile UMETA(DisplayName = "Projectile"),
	EAT_AOE UMETA(DisplayName = "Area Of Effect"),
	EAT_Instant UMETA(DisplayName = "Instant"),
	EAT_Buff UMETA(DisplayName = "Buff"),

	EAT_MAX UMETA(DisplayName = "DefaultMAX")
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

	//Powers
	Ability1		UMETA(DisplayName = "Ability1"),

	//Weapons
	WeaponEquip		UMETA(DisplayName = "WeaponEquip"),
	WeaponUnequip	UMETA(DisplayName = "WeaponUnequip"),
	AttackLight		UMETA(DisplayName = "AttackLight"),
	AttackHeavy		UMETA(DisplayName = "AttackHeavy"),
	Block			UMETA(DisplayName = "Block"),

	//Character
	Sprint			UMETA(DisplayName = "Sprint"),
	Dodge			UMETA(DisplayName = "Dodge"),
	Jump			UMETA(DisplayName = "Jump")
};

