// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGPingMarker.h"

#include "Components/WidgetComponent.h"
#include "Libraries/WOGBlueprintLibrary.h"
#include "UI/WOGPingWidget.h"


// Sets default values
AWOGPingMarker::AWOGPingMarker()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	SetReplicateMovement(false);

	PingWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("Ping Widget Component"));
	PingWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	PingWidgetComponent->SetDrawAtDesiredSize(true);
}

// Called when the game starts or when spawned
void AWOGPingMarker::BeginPlay()
{
	Super::BeginPlay();

	UWOGPingWidget* PingWidget = Cast<UWOGPingWidget>(PingWidgetComponent->GetUserWidgetObject());
	if(PingWidget)
	{
		PingWidget->InitPingWidget(UWOGBlueprintLibrary::GetCharacterData(GetOwner()).bIsAttacker, this);
	}
	
	SetLifeSpan(5.f);
}

