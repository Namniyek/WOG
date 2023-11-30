// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGPlayerController.h"
#include "WOG.h"
#include "GameInstance/WOGGameInstance.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Data/PlayerProfileSaveGame.h"
#include "PlayerCharacter/BasePlayerCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Enemies/WOGPossessableEnemy.h"
#include "Characters/WOGBaseCharacter.h"
#include "PlayerCharacter/WOGAttacker.h"
#include "AbilitySystemComponent.h"
#include "ActorComponents/WOGUIManagerComponent.h"
#include "Subsystems/WOGUIManagerSubsystem.h"

AWOGPlayerController::AWOGPlayerController()
{
	UIManagerComponent = CreateDefaultSubobject<UWOGUIManagerComponent>(TEXT("UI Manager Component"));
	UIManagerComponent->SetIsReplicated(true);
}

void AWOGPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWOGPlayerController, bIsAttacker);
}

void AWOGPlayerController::AcknowledgePossession(APawn* P)
{
	Super::AcknowledgePossession(P);

	AWOGBaseCharacter* CharacterBase = Cast<AWOGBaseCharacter>(P);
	if (CharacterBase)
	{
		CharacterBase->GetAbilitySystemComponent()->InitAbilityActorInfo(CharacterBase, CharacterBase);
	}
}

void AWOGPlayerController::OnPossess(APawn* aPawn)
{
	Super::OnPossess(aPawn);

	if (DefaultPawn) return;
	DefaultPawn = Cast<ABasePlayerCharacter>(aPawn);

	UWOGGameInstance* GameInstance = GetGameInstance<UWOGGameInstance>();
	if (!GameInstance)
	{
		return;
	}

	TArray <FString> PlayerNameArray;
	GameInstance->PlayersMap.GenerateValueArray(PlayerNameArray);
	FString DesiredPlayerName = PlayerState->GetPlayerName();
	UPlayerProfileSaveGame* WOGSavegame = nullptr;

	if (PlayerNameArray.IsEmpty()) return;

	for (int32 i = 0; i < PlayerNameArray.Num(); i++)
	{
		FString PlayerName = PlayerNameArray[i];
		if (PlayerName != DesiredPlayerName) continue;

		WOGSavegame = Cast<UPlayerProfileSaveGame>(UGameplayStatics::LoadGameFromSlot(PlayerName, i));
	}

	ABasePlayerCharacter* PlayerCharacter = Cast<ABasePlayerCharacter>(aPawn);
	if (!WOGSavegame || !PlayerCharacter) return;

	PlayerCharacter->Server_SetPlayerProfile(WOGSavegame->PlayerProfile);
	bIsAttacker = PlayerCharacter->GetPlayerProfile().bIsAttacker;
	SetPawn(PlayerCharacter);
	PlayerCharacter->SetOwnerPC(this);

	Server_SetPlayerIndex(WOGSavegame->PlayerProfile.UserIndex);

	UIManagerComponent->Client_ResetHUD();
}

void AWOGPlayerController::BeginPlay()
{
	Super::BeginPlay();

	//Init UIManagerSubsystem and variables
	TObjectPtr<UWOGUIManagerSubsystem> UIManager = ULocalPlayer::GetSubsystem<UWOGUIManagerSubsystem>(GetLocalPlayer());
	if (UIManager)
	{
		UIManager->InitVariables();
		UE_LOG(WOGLogUI, Display, TEXT("InitVariables called from WOGPlayerController class"));
	}
	else
	{
		UE_LOG(WOGLogUI, Error, TEXT("Invalid UI Manager subsystem from WOGPlayerController class"));
	}
}

void AWOGPlayerController::Server_PossessMinion_Implementation(AActor* ActorToPossess)
{
	TObjectPtr<AWOGAttacker> Attacker = Cast<AWOGAttacker>(GetPawn());
	if (Attacker)
	{
		Attacker->SetCurrentExternalMinion(ActorToPossess);
	}

	if (!ActorToPossess)
	{
		UE_LOG(LogTemp, Error, TEXT("ActorToPossess invalid"));
		return;
	}

	TObjectPtr<APawn> PawnToPossess = Cast<APawn>(ActorToPossess);
	if (!PawnToPossess)
	{
		UE_LOG(LogTemp, Error, TEXT("PawnToPossess invalid"));
		return;
	}

	if (PawnToPossess->IsPlayerControlled())
	{
		UE_LOG(LogTemp, Error, TEXT("ALREADY CONTROLLED BY ANOTHER PLAYER"));
		return;
	}

	FTimerHandle BlendTimer;
	FTimerDelegate BlendDelegate;
	BlendDelegate.BindUFunction(this, FName("Possess"), PawnToPossess);
	float BlendTime = 1.f;
	GetWorldTimerManager().SetTimer(BlendTimer, BlendDelegate, BlendTime, false);

	SetViewTargetWithBlend(ActorToPossess, BlendTime);

	UIManagerComponent->Client_RemoveBarsWidget();
	UIManagerComponent->Client_CollapseAbilitiesWidget();
	UIManagerComponent->Client_RemoveAvailableResourceWidget();
}

void AWOGPlayerController::Server_UnpossessMinion_Implementation(APawn* AIPawnLeft)
{
	if (!DefaultPawn)
	{
		UE_LOG(LogTemp, Error, TEXT("DefaultPawn invalid"));
		return;
	}
	if (!AIPawnLeft)
	{
		UE_LOG(LogTemp, Error, TEXT("AIPawnLeft invalid"));
		return;
	}

	FTimerHandle BlendTimer;
	FTimerDelegate BlendDelegate;
	BlendDelegate.BindUFunction(this, FName("FinishUnPossess"), DefaultPawn, AIPawnLeft);
	float BlendTime = 0.5f;
	GetWorldTimerManager().SetTimer(BlendTimer, BlendDelegate, BlendTime, false);

	SetViewTargetWithBlend(DefaultPawn, BlendTime, VTBlend_Linear, 0.f, true);
}

void AWOGPlayerController::Server_SetPlayerIndex_Implementation(int32 NewIndex)
{
	UserIndex = NewIndex;
}

void AWOGPlayerController::FinishUnPossess(APawn* PawnToPossess, APawn* AIPawnLeft)
{ 
	UnPossess();
	Possess(PawnToPossess);
	AIPawnLeft->SpawnDefaultController();

	TObjectPtr<AWOGAttacker> Attacker = Cast<AWOGAttacker>(PawnToPossess);
	if (Attacker)
	{
		Attacker->SetCurrentExternalMinion(nullptr);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid attacker for resetting CurrentExternalMinion"));
	}

	UIManagerComponent->Client_ResetHUD();
}
