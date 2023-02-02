#pragma once

UENUM(BlueprintType)
enum class ECharacterState : uint8
{
	ECS_Unnoccupied UMETA(DisplayName = "Unnoccupied"),
	ECS_Dodging UMETA(DisplayName = "Dodging"),
	ECS_Sprinting UMETA(DisplayName = "Sprinting"),
	ECS_Targeting UMETA(DisplayName = "Targeting"),

	ECS_MAX UMETA(DisplayName = "DefaultMAX")
};