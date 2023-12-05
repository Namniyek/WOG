#pragma once
#include "CoreMinimal.h"
#include "GameplayTags.h"
#include "GameplayTagContainer.h"
#include "Types/WOGGameplayTags.h"
#include "WOGDataTypes.generated.h"

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
enum class EItemType : uint8
{
	EIT_Weapon UMETA(DisplayName = "Weapon"),
	EIT_Magic UMETA(DisplayName = "Magic"),
	EIT_Consumable UMETA(DisplayName = "Consumable"),

	EIT_MAX UMETA(DisplayName = "DefaultMAX")
};

UENUM(BlueprintType)
enum class EAbilityInputType : uint8
{
	EAI_Instant UMETA(DisplayName = "Instant"),
	EAI_Hold UMETA(DisplayName = "Hold"),

	EAI_MAX UMETA(DisplayName = "DefaultMAX")
};

UENUM(BlueprintType)
enum class EDayNPCState : uint8
{
	EDNS_Idle UMETA(DisplayName = "Idle"),
	EDNS_Working UMETA(DisplayName = "Working"),
	EDNS_Resting UMETA(DisplayName = "Resting"),
	EDNS_Return UMETA(DisplayName = "Returning Home"),

	EDNS_MAX UMETA(DisplayName = "DefaultMAX")
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
struct FCostMap
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cost Data")
	FGameplayTag CostTag = FGameplayTag();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Cost Data")
	int32 CostAmount = 0;
};

USTRUCT(BlueprintType)
struct FVendorItemData
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BaseData")
	EItemType ItemType = EItemType::EIT_MAX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BaseData")
	FName DisplayName = FName("Empty");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BaseData")
	FName ItemDescription = FName("Empty");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BaseData")
	FGameplayTag ItemTag = FGameplayTag();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BaseData")
	UTexture2D* ItemIcon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BaseData")
	TSubclassOf<AActor> ItemClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BaseData")
	int32 ItemAmount = 1;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "BaseData")
	TArray<FCostMap> CostMap = {};

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "BaseData")
	TSubclassOf<class UWOGInspectItemBaseWidget> InspectWidgetClass = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BaseData")
	bool bIsAttacker = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (EditCondition = "ItemType == EItemType::EIT_Weapon", EditConditionHides))
	float BaseWeaponDamage = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (EditCondition = "ItemType == EItemType::EIT_Weapon", EditConditionHides))
	float ComboMultiplier = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (EditCondition = "ItemType == EItemType::EIT_Weapon", EditConditionHides))
	float HeavyDamageMultiplier = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (EditCondition = "ItemType == EItemType::EIT_Weapon", EditConditionHides))
	float WeaponStaminaCost = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon", meta = (EditCondition = "ItemType == EItemType::EIT_Weapon", EditConditionHides))
	FName WeaponRangedAttackName = FName("Empty");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic", meta = (EditCondition = "ItemType == EItemType::EIT_Magic", EditConditionHides))
	FName TypeOfMagic = FName("Empty");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Magic", meta = (EditCondition = "ItemType == EItemType::EIT_Magic", EditConditionHides))
	float BaseMagicValue = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Magic", meta = (EditCondition = "ItemType == EItemType::EIT_Magic", EditConditionHides))
	float MagicCooldown = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic", meta = (EditCondition = "ItemType == EItemType::EIT_Magic", EditConditionHides))
	FName EffectApplied = FName("Empty");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Magic", meta = (EditCondition = "ItemType == EItemType::EIT_Magic", EditConditionHides))
	float MagicDuration = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Magic", meta = (EditCondition = "ItemType == EItemType::EIT_Magic", EditConditionHides))
	float CastCost = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consumable", meta = (EditCondition = "ItemType == EItemType::EIT_Consumable", EditConditionHides))
	FName TypeOfConsumable = FName("Empty");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consumable", meta = (EditCondition = "ItemType == EItemType::EIT_Consumable", EditConditionHides))
	FName AffectedAttribute = FName("Empty");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Consumable", meta = (EditCondition = "ItemType == EItemType::EIT_Consumable", EditConditionHides))
	float BaseConsumableValue = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Consumable", meta = (EditCondition = "ItemType == EItemType::EIT_Consumable", EditConditionHides))
	float ConsumableDuration = 0.f;
};