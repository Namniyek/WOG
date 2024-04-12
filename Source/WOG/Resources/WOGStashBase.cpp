// Fill out your copyright notice in the Description page of Project Settings.


#include "Resources/WOGStashBase.h"
#include "WOG.h"
#include "Components/SphereComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/WidgetComponent.h"
#include "Data/AGRLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "Resources/WOGCommonInventory.h"
#include "PlayerCharacter/BasePlayerCharacter.h"
#include "Subsystems/WOGUIManagerSubsystem.h"
#include "Subsystems/WOGWorldSubsystem.h"

AWOGStashBase::AWOGStashBase()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	OverlapSphere = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapSphere"));
	SetRootComponent(OverlapSphere);
	OverlapSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	OverlapSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	OverlapSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(GetRootComponent());

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(GetRootComponent());

	InteractWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("Interact Widget"));
	InteractWidget->SetupAttachment(Mesh);

	BusyWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("Busy Widget"));
	BusyWidget->SetupAttachment(Mesh);

	CommonInventoryTag = FName("Defender");

}

void AWOGStashBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWOGStashBase, CommonInventory);
	DOREPLIFETIME(AWOGStashBase, bIsBusy);
	DOREPLIFETIME(AWOGStashBase, PlayerUsingStash);
	DOREPLIFETIME(AWOGStashBase, OverlappingPlayers);
	DOREPLIFETIME(AWOGStashBase, bIsDay);
}

void AWOGStashBase::BeginPlay()
{
	Super::BeginPlay();

	if (OverlapSphere)
	{
		OverlapSphere->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnSphereBeginOverlap);
		OverlapSphere->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnSphereEndOverlap);
	}

	InteractWidget->SetHiddenInGame(true);
	BusyWidget->SetHiddenInGame(true);

	if (HasAuthority())
	{
		TArray<AActor*> OutItems = {};
		UGameplayStatics::GetAllActorsOfClassWithTag(this, AWOGCommonInventory::StaticClass(), CommonInventoryTag, OutItems);

		if (OutItems.IsEmpty() || !OutItems[0]) return;

		CommonInventory = UAGRLibrary::GetInventory(OutItems[0]);

		TObjectPtr<UWOGWorldSubsystem> WorldSubsystem = GetWorld()->GetSubsystem<UWOGWorldSubsystem>();
		if (WorldSubsystem)
		{
			WorldSubsystem->OnKeyTimeHitDelegate.AddDynamic(this, &ThisClass::OnKeyTimeHit);
			WorldSubsystem->TimeOfDayChangedDelegate.AddDynamic(this, &ThisClass::TimeOfDayChanged);
		}
	}

}

void AWOGStashBase::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	TObjectPtr<ABasePlayerCharacter> Player = Cast<ABasePlayerCharacter>(OtherActor);
	if (!Player) return;

	if (Player->IsLocallyControlled())
	{
		Player->OnInteractComplete.AddDynamic(this, &ThisClass::OnInteractWithStashComplete);
	}

	if (Player->HasAuthority())
	{
		OverlappingPlayers.AddUnique(Player);
	}

	if (bIsBusy)
	{
		if (Player->IsLocallyControlled())
		{
			BusyWidget->SetHiddenInGame(false);
		}
	}
	else
	{
		if (Player->IsLocallyControlled())
		{
			InteractWidget->SetHiddenInGame(false);
		}
	}
}

void AWOGStashBase::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor) return;

	//Clear the interact delegate and hide all widgets
	TObjectPtr<ABasePlayerCharacter> Player = Cast<ABasePlayerCharacter>(OtherActor);
	if (Player && Player->IsLocallyControlled())
	{
		Player->OnInteractComplete.Clear();
		BusyWidget->SetHiddenInGame(true);
		InteractWidget->SetHiddenInGame(true);
	}

	//Reverse for loop to remove OtherActor from OverlappingPlayers array. 
	if (!OtherActor->HasAuthority()) return;
	for (int32 i = OverlappingPlayers.Num() - 1; i >= 0; i--)
	{
		Player = OverlappingPlayers[i];
		if (Player && Player == OtherActor)
		{
			OverlappingPlayers.Remove(Player);
		}
	}
}

void AWOGStashBase::SetIsBusy(const bool& NewBusy, ABasePlayerCharacter* UserPlayer)
{
	if (!HasAuthority()) return;
	PlayerUsingStash = UserPlayer;
	bIsBusy = NewBusy;

	ShowCorrectWidget(bIsBusy, PlayerUsingStash);
	HandleCosmetics(bIsBusy);
}

void AWOGStashBase::BackFromWidget_Implementation(AActor* Actor)
{
	FreeStash();
}

void AWOGStashBase::SwitchItem_Implementation(const bool bToCommon, AActor* ItemToSwitch, AActor* PreviousItem, const FGameplayTagContainer AuxTagsContainer, const TSubclassOf<AActor> ItemClass, const int32& Amount)
{
	if (!PlayerUsingStash) return;
	PlayerUsingStash->Server_SwitchItem(this, bToCommon, ItemToSwitch, PreviousItem, AuxTagsContainer, ItemClass, Amount);
}

void AWOGStashBase::SwitchStashedItems(const bool& bToCommon, AActor* ItemToSwitch, AActor* PreviousItem, FGameplayTagContainer AuxTagsContainer, const TSubclassOf<AActor> ItemClass, const int32& Amount)
{
	if (!HasAuthority()) return;
	if (!CommonInventory) return;
	if (!PlayerUsingStash) return;
	if (!ItemToSwitch) return;

	bool bIsSwitch = PreviousItem != nullptr;

	TObjectPtr<UAGR_ItemComponent> Item = UAGRLibrary::GetItemComponent(ItemToSwitch);
	if (!Item) return;

	bool bIsSpawnMenuSwitch = (Item->ItemTagSlotType.MatchesTag(TAG_Inventory_Buildable) || Item->ItemTagSlotType.MatchesTag(TAG_Inventory_Spawnable));

	TObjectPtr<UAGR_InventoryManager> Recipient = bToCommon ? CommonInventory : (TObjectPtr<UAGR_InventoryManager>) UAGRLibrary::GetInventory(PlayerUsingStash);
	if (!Recipient) return;

	TObjectPtr<UAGR_InventoryManager> Giver = bToCommon ? (TObjectPtr<UAGR_InventoryManager>) UAGRLibrary::GetInventory(PlayerUsingStash) : CommonInventory;
	
	if (Item->InventoryId == Recipient->InventoryId)
	{
		UE_LOG(WOGLogInventory, Error, TEXT("Trying to move item into the same inventory. Returning"));
		return;
	}

	if (bIsSwitch && Giver)
	{
		if (ItemToSwitch == PreviousItem) return;

		TObjectPtr<UAGR_ItemComponent> SwitchItem = UAGRLibrary::GetItemComponent(PreviousItem);
		if (SwitchItem && !SwitchItem->bStackable)
		{
			if (!bIsSpawnMenuSwitch || Recipient == CommonInventory)
			{
				SwitchItem->DropItem();
				SwitchItem->PickUpItem(Giver);
				SwitchItem->ItemAuxTag = AuxTagsContainer.First();
			}
		}
		if (SwitchItem && SwitchItem->bStackable)
		{
			FText OutNote;
			int32 AmountToModify = SwitchItem->CurrentStack < Amount ? SwitchItem->CurrentStack : Amount;
			Giver->AddItemsOfClass(PreviousItem->GetClass(), SwitchItem->CurrentStack, OutNote);
			Recipient->RemoveItemsWithTagSlotType(SwitchItem->ItemTagSlotType, SwitchItem->CurrentStack, OutNote);
		}
	}

	if (!Item->bStackable)
	{
		if (bIsSpawnMenuSwitch && Giver == CommonInventory)
		{
			FText OutNote;
			Recipient->AddItemsOfClass(ItemClass, 1, OutNote);
			return;
		}
		if (bIsSpawnMenuSwitch && Recipient == CommonInventory && Giver != CommonInventory)
		{
			Item->DropItem();
			Item->GetOwner()->Destroy();
			return;
		}

		Item->DropItem();
		Item->PickUpItem(Recipient);
		Item->ItemAuxTag = AuxTagsContainer.First();
		Item->PreviousOwnerIndex = -1;

	}
	if (Item->bStackable && Giver)
	{
		FText OutNote;
		int32 AmountToModify = Item->CurrentStack < Amount ? Item->CurrentStack : Amount;

		Recipient->AddItemsOfClass(ItemClass, AmountToModify, OutNote);

		if (bIsSpawnMenuSwitch && Giver == CommonInventory) return;

		Giver->RemoveItemsOfClass(ItemClass, AmountToModify, OutNote);
		UE_LOG(WOGLogInventory, Display, TEXT("%d of %s sent from %s to %s - NO SWITCH"), AmountToModify, *GetNameSafe(ItemClass), *GetNameSafe(Giver), *GetNameSafe(Recipient));
	}

}

void AWOGStashBase::OnRep_IsBusy()
{
	if (HasAuthority()) return;

	ShowCorrectWidget(bIsBusy, PlayerUsingStash);
	HandleCosmetics(bIsBusy);
}

void AWOGStashBase::OnInteractWithStashComplete(ABasePlayerCharacter* Interactor)
{
	for (ABasePlayerCharacter* Player : OverlappingPlayers)
	{
		if (Player && Player == Interactor && !bIsBusy)
		{
			Player->Server_SetStashBusy(true, Player, this);

			if (!Player->IsLocallyControlled()) return;

			CurrentUserPC = Cast<APlayerController>(Player->GetController());
			if (!CurrentUserPC) return;

			CurrentUserPC->SetViewTargetWithBlend(this, 0.5f);

			TObjectPtr<UWOGUIManagerSubsystem> UIManager = ULocalPlayer::GetSubsystem<UWOGUIManagerSubsystem>(CurrentUserPC->GetLocalPlayer());
			if (IsValid(UIManager))
			{
				UIManager->RemoveStaminaWidget();
				UIManager->RemoveBarsWidget();
				UIManager->RemoveMinimapWidget();
				UIManager->CollapseAbilitiesWidget();
				UIManager->CollapseObjectiveWidget();
				UIManager->RemoveAvailableResourcesWidget();
			}

			FTimerHandle ViewBlendTimerHandle;

			GetWorldTimerManager().SetTimer(ViewBlendTimerHandle, this, &ThisClass::OnCameraBlendInFinished, 0.5f);

			Player->GetMesh()->SetHiddenInGame(true);

			TArray<AActor*> OutActors = {};
			Player->GetAttachedActors(OutActors, true, true);
			for (auto Actor : OutActors)
			{
				Actor->SetActorHiddenInGame(true);
			}
		}
	}
}

void AWOGStashBase::OnCameraBlendInFinished()
{
	TObjectPtr<UWOGUIManagerSubsystem> UIManager = ULocalPlayer::GetSubsystem<UWOGUIManagerSubsystem>(CurrentUserPC->GetLocalPlayer());
	if (UIManager)
	{
		UIManager->AddStashWidget(PlayerUsingStash, this);
	}

	if (CurrentUserPC)
	{
		FInputModeUIOnly InputMode;
		CurrentUserPC->SetInputMode(InputMode);
		CurrentUserPC->bShowMouseCursor = true;
	}
}

void AWOGStashBase::FreeStash()
{
	if (!PlayerUsingStash) return;
	if (!PlayerUsingStash->IsLocallyControlled()) return;

	if (CurrentUserPC)
	{
		CurrentUserPC->SetViewTargetWithBlend(PlayerUsingStash, 0.5f);

		TObjectPtr<UWOGUIManagerSubsystem> UIManager = ULocalPlayer::GetSubsystem<UWOGUIManagerSubsystem>(CurrentUserPC->GetLocalPlayer());
		if (UIManager)
		{
			UIManager->RemoveStashWidget();
			UIManager->ResetHUD();
		}

		FInputModeGameOnly InputMode;
		CurrentUserPC->SetInputMode(InputMode);
		CurrentUserPC->bShowMouseCursor = false;

		CurrentUserPC = nullptr;

		FTimerHandle ViewBlendTimerHandle;
		GetWorldTimerManager().SetTimer(ViewBlendTimerHandle, this, &ThisClass::OnCameraBlendOutFinished, 0.25f);
	}
}

void AWOGStashBase::OnCameraBlendOutFinished()
{
	if (PlayerUsingStash && PlayerUsingStash->GetMesh())
	{
		PlayerUsingStash->GetMesh()->SetHiddenInGame(false);
		TArray<AActor*> OutActors = {};
		PlayerUsingStash->GetAttachedActors(OutActors, true, true);
		for (auto Actor : OutActors)
		{
			Actor->SetActorHiddenInGame(false);
		}

		if (bIsDay)
		{
			PlayerUsingStash->Server_SetStashBusy(false, PlayerUsingStash, this);
		}
		if (!bIsDay)
		{
			PlayerUsingStash->Server_SetStashBusy(true, PlayerUsingStash, this);
		}
	}
}

void AWOGStashBase::ShowCorrectWidget(const bool bIsVendorBusy, ABasePlayerCharacter* OverlappingActor)
{
	if (!OverlappingActor) return;

	for (auto Player : OverlappingPlayers)
	{
		if (!Player) break;
		if (bIsVendorBusy)
		{
			if (Player->IsLocallyControlled())
			{
				InteractWidget->SetHiddenInGame(true);
				if (Player != OverlappingActor)
				{
					BusyWidget->SetHiddenInGame(false);
				}
				else
				{
					BusyWidget->SetHiddenInGame(true);
				}
			}
		}
		else
		{
			if (Player->IsLocallyControlled())
			{
				BusyWidget->SetHiddenInGame(true);
				InteractWidget->SetHiddenInGame(false);
			}
		}
	}
}

void AWOGStashBase::RefreshStashItems()
{

}

void AWOGStashBase::TimeOfDayChanged(const ETimeOfDay TOD)
{
	switch (TOD)
	{
	case ETimeOfDay::TOD_Dawn2:
		SetIsBusy(false, nullptr);
		bIsDay = true;
		break;
	case ETimeOfDay::TOD_Dawn3:
		SetIsBusy(false, nullptr);
		bIsDay = true;
		break;
	default:
		break;
	}
}

void AWOGStashBase::OnKeyTimeHit(const int32 CurrentTime)
{
	if (CurrentTime == 1070)
	{
		bIsDay = false;
		if (PlayerUsingStash)
		{
			PlayerUsingStash->Client_KickPlayerFromStash(this);
		}
		else
		{
			SetIsBusy(true, nullptr);
			ShowCorrectWidget(true, nullptr);
		}
	}
}

