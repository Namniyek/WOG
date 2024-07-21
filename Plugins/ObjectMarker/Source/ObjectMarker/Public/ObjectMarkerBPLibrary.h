// Copyright BabbyGames All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "SceneView.h"
#include "Blueprint/UserWidget.h"
#include "Components/Slider.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/ActorComponent.h"
#include "SlateBasics.h"
#include "SlateCore.h"
#include "UnrealClient.h"
#include "ObjectMarkerBPLibrary.generated.h"

/* 
*	Function library class.
*	Each function in it is expected to be static and represents blueprint node that can be called in any blueprint.
*
*	When declaring function you can define metadata for the node. Key function specifiers will be BlueprintPure and BlueprintCallable.
*	BlueprintPure - means the function does not affect the owning object in any way and thus creates a node without Exec pins.
*	BlueprintCallable - makes a function which can be executed in Blueprints - Thus it has Exec pins.
*	DisplayName - full name of the node, shown when you mouse over the node and in the blueprint drop down menu.
*				Its lets you name the node using characters not allowed in C++ function names.
*	CompactNodeTitle - the word(s) that appear on the node.
*	Keywords -	the list of keywords that helps you to find node when you search for it using Blueprint drop-down menu. 
*				Good example is "Print String" node which you can find also by using keyword "log".
*	Category -	the category your node will be under in the Blueprint drop-down menu.
*
*	For more info on custom blueprint nodes visit documentation:
*	https://wiki.unrealengine.com/Custom_Blueprint_Node_Creation
*/
UCLASS()
class UObjectMarkerBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

public:
	//Projects WorldLocationToScreenPosition
	UFUNCTION(BlueprintCallable, Category = "ObjectMarker")
	static FVector2D ProjectWorldToScreen(APlayerController* PlayerController, const FVector& TargetLocation);
	
	//Caluculates Whether The Object Is Infront/OutSideOfScreen And Returns Position On Screen
	UFUNCTION(BlueprintPure, Category = "ObjectMarker")
	static bool CalculateObjectLocationOnScreen(APlayerController* Player, AActor const* TargetActor, FVector2D& ScreenPosition, bool& bTargetBehindCamera, bool bPlayerViewportRelative = false);
	
	//Controls OffScreenWidget
	UFUNCTION(BlueprintCallable, Category = "ObjectMarker")
	static void OffScreenIndicatorController(APlayerController* PlayerController, AActor* TargetActor, USlider* SliderUP, USlider* SliderDOWNRIGHT, USlider* SliderDOWNLEFT, USlider* SliderRIGHT, USlider* SliderLEFT,USlider* SliderDOWN);

	//Adds Objects Which Are Marked In A Array
	UFUNCTION(BlueprintCallable, Category = "ObjectMarker")
	static void AddIndicatedObjectsToArray(AActor* Actor);

	//Returns MarkedObjects In An Array
	UFUNCTION(BlueprintPure, Category = "ObjectMarker")
	static void GetIndicatedActors(TArray<AActor*>& MarkedActors);

	//Clears All Arrays And Variable In ObjectMarker
	UFUNCTION(BlueprintCallable, Category = "ObjectMarker")
	static void ResetObjectIndicator();

};
