// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGLobbyPawn.h"
#include "Camera/CameraComponent.h "

// Sets default values
AWOGLobbyPawn::AWOGLobbyPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetFieldOfView(100.f);
	SetRootComponent(Camera);
}

// Called when the game starts or when spawned
void AWOGLobbyPawn::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AWOGLobbyPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AWOGLobbyPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

