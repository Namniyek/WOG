// Fill out your copyright notice in the Description page of Project Settings.


#include "Resources/WOGResourceBase.h"
#include "Components/AGR_ItemComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Data/AGRLibrary.h"
#include "Enemies/WOGMinerGiant.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Resources/WOGCommonInventory.h"
#include "PlayerCharacter/BasePlayerCharacter.h"
#include "PlayerCharacter/WOGAttacker.h"
#include "Slate/SGameLayerManager.h"


// Sets default values
AWOGResourceBase::AWOGResourceBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	ItemComponent = CreateDefaultSubobject<UAGR_ItemComponent>(TEXT("ItemComponent"));
	ItemComponent->bStackable = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);

	OverlapSphere = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapSphere"));
	OverlapSphere->SetupAttachment(Mesh);
	OverlapSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	OverlapSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	OverlapSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	InteractWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("Interact Widget"));
	InteractWidget->SetupAttachment(GetRootComponent());
}

void AWOGResourceBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWOGResourceBase, OverlappingPlayers);
}

// Called when the game starts or when spawned
void AWOGResourceBase::BeginPlay()
{
	Super::BeginPlay();

	if (OverlapSphere)
	{
		OverlapSphere->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnSphereBeginOverlap);
		OverlapSphere->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnSphereEndOverlap);
	}

	InteractWidget->SetHiddenInGame(true);
}

void AWOGResourceBase::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(OtherActor && OtherActor->IsA<AWOGMinerGiant>())
	{
		AWOGAttacker* Attacker = Cast<AWOGAttacker>(UGameplayStatics::GetActorOfClass(this, AWOGAttacker::StaticClass()));
		if(Attacker)
		{
			Attacker->Server_CollectResource(ItemComponent);
			return;
		}
	}
	
	TObjectPtr<ABasePlayerCharacter> Player = Cast<ABasePlayerCharacter>(OtherActor);
	if (!Player) return;

	if (Player->IsLocallyControlled())
	{
		Player->OnInteractComplete.AddDynamic(this, &ThisClass::OnInteractWithResourceComplete);
		InteractWidget->SetHiddenInGame(false);
	}

	if (Player->HasAuthority())
	{
		OverlappingPlayers.AddUnique(Player);
	}
}

void AWOGResourceBase::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor) return;

	//Clear the interact delegate and hide all widgets
	TObjectPtr<ABasePlayerCharacter> Player = Cast<ABasePlayerCharacter>(OtherActor);
	if (Player && Player->IsLocallyControlled())
	{
		Player->OnInteractComplete.Clear();
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

void AWOGResourceBase::OnInteractWithResourceComplete(ABasePlayerCharacter* Interactor)
{
	if (!Interactor || !ItemComponent) return;

	Interactor->Server_CollectResource(ItemComponent);
}

