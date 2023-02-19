// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGAttacker.h"
#include "WOG/ActorComponents/WOGAttributesComponent.h"

void AWOGAttacker::BeginPlay()
{
	Super::BeginPlay();

	if (Attributes)
	{
		Attributes->Server_PassiveAttributeUpdate(EAttributeType::AT_Mana, 2.f);
	}
}
