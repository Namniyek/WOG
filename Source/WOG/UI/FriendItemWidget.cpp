// Fill out your copyright notice in the Description page of Project Settings.


#include "FriendItemWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "WOG/GameInstance/WOGGameInstance.h"

bool UFriendItemWidget::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}

	if (B_Invite)
	{
		B_Invite->OnClicked.AddDynamic(this, &ThisClass::SendInvite);
	}

	if(GEngine) GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Orange, FString("Initialized"));

	return true;
}
	

void UFriendItemWidget::SendInvite()
{
	GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Orange, FString("InvitationSent"));
	UWOGGameInstance * GameInstance = Cast< UWOGGameInstance>(GetGameInstance());

	if (GameInstance)
	{

	}
}
