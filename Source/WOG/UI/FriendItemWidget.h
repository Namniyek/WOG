// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FriendItemWidget.generated.h"

/**
 * 
 */
UCLASS()
class WOG_API UFriendItemWidget : public UUserWidget
{
	GENERATED_BODY()
	

public:

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	class UTextBlock* T_FriendName;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	class UButton* B_Invite;

	UFUNCTION()
	void SendInvite();

protected:

	virtual bool Initialize() override;
};
