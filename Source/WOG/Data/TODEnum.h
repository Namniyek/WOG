#pragma once

UENUM(BlueprintType)
enum class ETimeOfDay : uint8
{
	TOD_Start UMETA(DisplayName = "Start"),
	TOD_Dusk1 UMETA(DisplayName = "Dusk_1"),
	TOD_Dawn2 UMETA(DisplayName = "Dawn_2"),
	TOD_Dusk2 UMETA(DisplayName = "Dusk_2"),
	TOD_Dawn3 UMETA(DisplayName = "Dawn_3"),
	TOD_Dusk3 UMETA(DisplayName = "Dusk_3"),
	TOD_Dawn4 UMETA(DisplayName = "Dawn_4"),

	TOD_MAX UMETA(DisplayName = "DefaultMAX")
};