// Fill out your copyright notice in the Description page of Project Settings.

#include "Resources/WOGVendor.h"
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
#include "UI/Vendors/WOGVendorBaseWidget.h"
#include "Subsystems/WOGWorldSubsystem.h"

AWOGVendor::AWOGVendor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	OverlapSphere = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapSphere"));
	SetRootComponent(OverlapSphere);
	OverlapSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	OverlapSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	OverlapSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(GetRootComponent());

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(GetRootComponent());

	InteractWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("Interact Widget"));
	InteractWidget->SetupAttachment(Mesh);

	BusyWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("Busy Widget"));
	BusyWidget->SetupAttachment(Mesh);

	VendorInventory = CreateDefaultSubobject<UAGR_InventoryManager>(TEXT("Vendor inventory"));
	VendorInventory->SetIsReplicated(true);

	CommonInventoryTag = FName("Defender");
}

void AWOGVendor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWOGVendor, CommonInventory);
	DOREPLIFETIME(AWOGVendor, bIsBusy);
	DOREPLIFETIME(AWOGVendor, PlayerUsingVendor);
	DOREPLIFETIME(AWOGVendor, OverlappingPlayers);
	DOREPLIFETIME(AWOGVendor, bIsDay);
}

void AWOGVendor::BeginPlay()
{
	Super::BeginPlay();

	if (OverlapSphere)
	{
		OverlapSphere->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnSphereBeginOverlap);
		OverlapSphere->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnSphereEndOverlap);
	}

	InteractWidget->SetHiddenInGame(true);
	BusyWidget->SetHiddenInGame(true);

	PopulateInventory();

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

void AWOGVendor::PopulateInventory()
{
	TArray<TSubclassOf<AActor>> OutClasses;
	DefaultInventoryMap.GetKeys(OutClasses);

	if (OutClasses.IsEmpty()) return;

	for (auto Class : OutClasses)
	{
		FText OutNote;
		VendorInventory->AddItemsOfClass(Class, *DefaultInventoryMap.Find(Class), OutNote);
	}
}

void AWOGVendor::Sell(const TArray<FCostMap>& CostMap, TSubclassOf<AActor> ItemClass, const int32& Amount)
{
	if (!CommonInventory || !VendorInventory) return;

	for (FCostMap Cost : CostMap)
	{
		FText OutNote;
		CommonInventory->RemoveItemsWithTagSlotType(Cost.CostTag, Cost.CostAmount, OutNote);
	}

	FText OutNote;
	TArray<AActor*> OutItems;
	int32 AmountToAdd = Amount;
	VendorInventory->GetAllItemsOfClass(ItemClass, OutItems);

	if (OutItems.Num() && OutItems[0])
	{
		TObjectPtr<UAGR_ItemComponent> Item = UAGRLibrary::GetItemComponent(OutItems[0]);
		if (!Item)
		{
			return;
		}

		if (!Item->bStackable)
		{
			Item->DropItem();
			Item->PickUpItem(CommonInventory);
		}
		else
		{
			if (Item->CurrentStack > Amount)
			{
				VendorInventory->RemoveItemsOfClass(ItemClass, Amount, OutNote);
				UE_LOG(WOGLogInventory, Display, TEXT("Vendor inventory: %s"), *OutNote.ToString());
			}
			else if (Item->CurrentStack <= Amount)
			{
				AmountToAdd = Item->CurrentStack;
				Item->DestroyItem();
			}

			CommonInventory->AddItemsOfClass(ItemClass, AmountToAdd, OutNote);
		}
	}

	FTimerHandle DelayTimer;
	GetWorldTimerManager().SetTimer(DelayTimer, this, &ThisClass::RefreshVendorItems, 0.05f, false);
}

void AWOGVendor::RefreshVendorItems()
{
	TObjectPtr<IInventoryInterface> Interface = Cast<IInventoryInterface>(PlayerUsingVendor);
	if (Interface)
	{
		Interface->Execute_TransactionComplete(PlayerUsingVendor);
	}
}

void AWOGVendor::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	TObjectPtr<ABasePlayerCharacter> Player = Cast<ABasePlayerCharacter>(OtherActor);
	if (!Player) return;

	if(Player->IsLocallyControlled())
	{
		Player->OnInteractComplete.AddDynamic(this, &ThisClass::OnInteractWithVendorComplete);
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
		if(Player->IsLocallyControlled())
		{
			InteractWidget->SetHiddenInGame(false);
		}
	}
}

void AWOGVendor::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
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
	for (int32 i = OverlappingPlayers.Num()-1; i >= 0; i--)
	{
		Player = OverlappingPlayers[i];
		if (Player && Player == OtherActor)
		{
			OverlappingPlayers.Remove(Player);
		}
	}
}

void AWOGVendor::OnInteractWithVendorComplete(ABasePlayerCharacter* Interactor)
{
	for (ABasePlayerCharacter* Player : OverlappingPlayers)
	{
		if (Player && Player == Interactor && !bIsBusy)
		{
			Player->Server_SetVendorBusy(true, Player, this);

			if (!Player->IsLocallyControlled()) return;

			CurrentUserPC = Cast<APlayerController>(Player->GetController());
			if (!CurrentUserPC) return;

			CurrentUserPC->SetViewTargetWithBlend(this, 0.5f);

			TObjectPtr<UWOGUIManagerSubsystem> UIManager = ULocalPlayer::GetSubsystem<UWOGUIManagerSubsystem>(CurrentUserPC->GetLocalPlayer());
			if (UIManager)
			{
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

void AWOGVendor::OnCameraBlendInFinished()
{
	TObjectPtr<UWOGUIManagerSubsystem> UIManager = ULocalPlayer::GetSubsystem<UWOGUIManagerSubsystem>(CurrentUserPC->GetLocalPlayer());
	if (UIManager)
	{
		UIManager->AddVendorWidget(PlayerUsingVendor, this);
	}

	if (CurrentUserPC)
	{
		FInputModeUIOnly InputMode;
		CurrentUserPC->SetInputMode(InputMode);
		CurrentUserPC->bShowMouseCursor = true;
	}
}

void AWOGVendor::BackFromWidget_Implementation(AActor* Actor)
{
	if (!Actor) return;
	FreeVendor();
}

void AWOGVendor::FreeVendor()
{
	if (!PlayerUsingVendor) return;
	if (!PlayerUsingVendor->IsLocallyControlled()) return;

	if (CurrentUserPC)
	{
		CurrentUserPC->SetViewTargetWithBlend(PlayerUsingVendor, 0.5f);

		TObjectPtr<UWOGUIManagerSubsystem> UIManager = ULocalPlayer::GetSubsystem<UWOGUIManagerSubsystem>(CurrentUserPC->GetLocalPlayer());
		if (UIManager)
		{
			UIManager->RemoveVendorWidget();
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

void AWOGVendor::OnCameraBlendOutFinished()
{
	if (PlayerUsingVendor && PlayerUsingVendor->GetMesh())
	{
		PlayerUsingVendor->GetMesh()->SetHiddenInGame(false);
		TArray<AActor*> OutActors = {};
		PlayerUsingVendor->GetAttachedActors(OutActors, true, true);
		for (auto Actor : OutActors)
		{
			Actor->SetActorHiddenInGame(false);
		}

		if (bIsDay)
		{
			PlayerUsingVendor->Server_SetVendorBusy(false, PlayerUsingVendor, this);
		}
		if (!bIsDay)
		{
			PlayerUsingVendor->Server_SetVendorBusy(true, PlayerUsingVendor, this);
		}
	}
}

void AWOGVendor::SetIsBusy(const bool& NewBusy, ABasePlayerCharacter* UserPlayer)
{
	if (!HasAuthority()) return;
	PlayerUsingVendor = UserPlayer;
	bIsBusy = NewBusy;

	ShowCorrectWidget(bIsBusy, PlayerUsingVendor);
}

void AWOGVendor::OnRep_IsBusy()
{
	if (HasAuthority()) return;

	ShowCorrectWidget(bIsBusy, PlayerUsingVendor);
}

void AWOGVendor::ShowCorrectWidget(bool bIsVendorBusy, ABasePlayerCharacter* OverlappingActor)
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

void AWOGVendor::TimeOfDayChanged(ETimeOfDay TOD)
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
	}
}

void AWOGVendor::OnKeyTimeHit(int32 CurrentTime)
{
	if (CurrentTime == 1070)
	{
		bIsDay = false;
		if (PlayerUsingVendor)
		{
			PlayerUsingVendor->Client_KickPlayerFromVendor(this);
		}
		else
		{
			SetIsBusy(true, nullptr);
			ShowCorrectWidget(true, nullptr);
		}
	}
}
