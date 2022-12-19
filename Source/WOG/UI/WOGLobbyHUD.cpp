// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGLobbyHUD.h"

AWOGLobbyHUD::AWOGLobbyHUD()
{

}

void AWOGLobbyHUD::BeginPlay()
{
	Super::BeginPlay();

	/*APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController)
	{
		FInputModeUIOnly UIOnly;
		PlayerController->SetInputMode(UIOnly);
		PlayerController->SetShowMouseCursor(true);
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, FString("UIOnly"));
	}*/
}
