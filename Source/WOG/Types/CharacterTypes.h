#pragma once

UENUM(BlueprintType)
enum class ECharacterState : uint8
{
	ECS_Unnoccupied UMETA(DisplayName = "Unnoccupied"),
	ECS_Dodging UMETA(DisplayName = "Dodging"),
	ECS_Sprinting UMETA(DisplayName = "Sprinting"),
	ECS_Attacking UMETA(DisplayName = "Attacking"),
	ECS_Blocking UMETA(DisplayName = "Blocking"),
	ECS_Staggered UMETA(DisplayName = "Staggered"),
	ECS_Elimmed UMETA(DisplayName = "Eliminated"),

	ECS_MAX UMETA(DisplayName = "DefaultMAX")
};

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_Unarmed UMETA(DisplayName = "Unarmed"),
	EWT_SwordAndShield UMETA(DisplayName = "Sword and Shield"),
	EWT_DoubleHanded UMETA(DisplayName = "Double Handed"),
	EWT_DualWielding UMETA(DisplayName = "Dual Wielding"),
	EWT_BowAndArrow UMETA(DisplayName = "BowAndArrow"),

	EWT_MAX UMETA(DisplayName = "DefaultMAX")
};

//namespace WeaponName
//{
//	const FName SwordAndShield = FName(TEXT("SwordAndShield"));
//	const FName AxeAndShield = FName(TEXT("AxeAndShield"));
//
//}

