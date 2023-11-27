// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/KismetSystemLibrary.h"
#include "PlayerCharacter/BasePlayerCharacter.h"
#include "WOG/ActorComponents/WOGAbilitySystemComponent.h"

AWOGPlayerState::AWOGPlayerState()
{
	PlayerStats.TimesElimmed = 0;
	PlayerStats.TotalElimms = 0;
	PlayerStats.MostElimmedPlayer = FString("DEFAULT");
	PlayerStats.PlayerWithMostElimms = FString("DEFAULT");

}

void AWOGPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWOGPlayerState, PlayerStats);
}

void AWOGPlayerState::IncreaseTimesElimmed()
{
	PlayerStats.TimesElimmed++;
}

void AWOGPlayerState::IncreaseTotalElimms()
{
	PlayerStats.TotalElimms++;
}

void AWOGPlayerState::SetMostElimmedPlayer(FString Player)
{
	PlayerStats.MostElimmedPlayer = Player;
}

void AWOGPlayerState::SetPlayerWithMostElimms(FString Player)
{
	PlayerStats.PlayerWithMostElimms = Player;
}

void AWOGPlayerState::RestoreEquipmentFromSnapshot()
{
	TObjectPtr<ABasePlayerCharacter> Character = Cast<ABasePlayerCharacter>(GetPawn());
	if(!IsValid(Character)) return; 

	FText OutNote;
	if (UKismetSystemLibrary::IsValidClass(EquipmentSnapshot.ClassSlotOne))
	{
		AActor* OutItemOne;
		Character->InventoryManager->AddItemsOfClassWithOutItem(EquipmentSnapshot.ClassSlotOne, 1, OutNote, OutItemOne);
		if (IsValid(OutItemOne))
		{
			TObjectPtr<UAGR_ItemComponent> Item = UAGRLibrary::GetItemComponent(OutItemOne);
			if (Item)
			{
				Item->ItemAuxTag = TAG_Aux_Weapon_Primary;
			}
		}
	}

	if (UKismetSystemLibrary::IsValidClass(EquipmentSnapshot.ClassSlotTwo))
	{
		AActor* OutItemTwo;
		Character->InventoryManager->AddItemsOfClassWithOutItem(EquipmentSnapshot.ClassSlotTwo, 1, OutNote, OutItemTwo);
		if (IsValid(OutItemTwo))
		{
			TObjectPtr<UAGR_ItemComponent> Item = UAGRLibrary::GetItemComponent(OutItemTwo);
			if (Item)
			{
				Item->ItemAuxTag = Character->GetCharacterData().bIsAttacker ? TAG_Aux_Magic_Primary : TAG_Aux_Weapon_Secondary;
			}
		}
	}

	if (UKismetSystemLibrary::IsValidClass(EquipmentSnapshot.ClassSlotThree))
	{
		AActor* OutItemThree;
		Character->InventoryManager->AddItemsOfClassWithOutItem(EquipmentSnapshot.ClassSlotThree, 1, OutNote, OutItemThree);
		if (IsValid(OutItemThree))
		{
			TObjectPtr<UAGR_ItemComponent> Item = UAGRLibrary::GetItemComponent(OutItemThree);
			if (Item)
			{
				Item->ItemAuxTag = Character->GetCharacterData().bIsAttacker ? TAG_Aux_Magic_Secondary : TAG_Aux_Magic_Primary;
			}
		}
	}

	if (UKismetSystemLibrary::IsValidClass(EquipmentSnapshot.ClassSlotFour))
	{
		AActor* OutItemFour = nullptr;;
		Character->InventoryManager->AddItemsOfClassWithOutItem(EquipmentSnapshot.ClassSlotFour, 1, OutNote, OutItemFour);
		if (IsValid(OutItemFour))
		{
			TObjectPtr<UAGR_ItemComponent> Item = UAGRLibrary::GetItemComponent(OutItemFour);
			if (Item)
			{
				Item->CurrentStack = EquipmentSnapshot.SlotFourAmount;
			}
		}
	}

	Character->RestoreEquipment();
}
