// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGPlayerController.h"
#include "WOG.h"
#include "GameInstance/WOGGameInstance.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Data/PlayerProfileSaveGame.h"
#include "PlayerCharacter/BasePlayerCharacter.h"
#include "WOG/UI/WOGMatchHUD.h"
#include "Blueprint/UserWidget.h"
#include "UI/MainAnnouncementWidget.h"
#include "Data/TODEnum.h"
#include "UI/EndgameWidget.h"
#include "Net/UnrealNetwork.h"
#include "Enemies/WOGPossessableEnemy.h"
#include "Characters/WOGBaseCharacter.h"
#include "PlayerCharacter/WOGAttacker.h"
#include "AbilitySystemComponent.h"
#include "UI/WOGAbilityWidget.h"
#include "UI/WOGAbilityContainerWidget.h"
#include "Components/Image.h"
#include "UI/WOG_HUD.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/VerticalBox.h"
#include "Components/Overlay.h"
#include "UI/WOGWarningWidget.h"
#include "UI/WOGObjectiveWidget.h"
#include "UI/WOGCharacterWidgetContainer.h"
#include "UI/WOGRoundProgressBar.h"
#include "Components/SizeBox.h"
#include "UI/WOGHoldProgressBar.h"
#include "UI/WOGRavenMarkerWidget.h"
#include "UI/WOGScreenDamage.h"
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
	bIsAttacker = PlayerCharacter->PlayerProfile.bIsAttacker;
	SetPawn(PlayerCharacter);
	PlayerCharacter->SetOwnerPC(this);

	Server_SetPlayerIndex(WOGSavegame->PlayerProfile.UserIndex);

	Client_ResetHUD();
}

void AWOGPlayerController::BeginPlay()
{
	Super::BeginPlay();
	MatchHUD = Cast<AWOGMatchHUD>(GetHUD());

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

void AWOGPlayerController::Client_CreateAnnouncementWidget_Implementation(ETimeOfDay NewTOD)
{
	MatchHUD == nullptr ? Cast<AWOGMatchHUD>(GetHUD()) : MatchHUD;
	if (!MatchHUD || !MatchHUD->AnnouncementClass) return;

	TOD = NewTOD;
	FString StringMain = FString();
	FString StringSec = FString();
	SetTODString(TOD, StringMain, StringSec);

	MatchHUD->AddAnnouncementWidget(StringMain, StringSec);
}

void AWOGPlayerController::Client_CreateAbilityWidget_Implementation(const int32& AbilityID, TSubclassOf<UUserWidget> Class, UTexture2D* Icon, const float& Cooldown, const FGameplayTag& Tag)
{
	MatchHUD == nullptr ? Cast<AWOGMatchHUD>(GetHUD()) : MatchHUD;
	if (!MatchHUD || !MatchHUD->HUDWidget) return;

	TObjectPtr<UWOGAbilityWidget> AbilityWidget = Cast<UWOGAbilityWidget>(CreateWidget<UUserWidget>(this, Class));
	if (!AbilityWidget) return;

	AbilityWidget->SetIconTexture(Icon);
	AbilityWidget->SetCooldownTag(Tag);
	AbilityWidget->SetCooldownTime(Cooldown);
	AbilityWidget->InitializeWidget();

	TObjectPtr<UWOGAbilityContainerWidget> Container = MatchHUD->HUDWidget->GetAbilityContainer();
	if (!Container) return;

	Container->AddChildAbility(AbilityID, AbilityWidget);
}

void AWOGPlayerController::Client_RemoveAbilityWidget_Implementation(const int32& AbilityID)
{
	MatchHUD == nullptr ? Cast<AWOGMatchHUD>(GetHUD()) : MatchHUD;
	if (!MatchHUD || !MatchHUD->HUDWidget) return;
	TObjectPtr<UWOGAbilityContainerWidget> Container = MatchHUD->HUDWidget->GetAbilityContainer();
	if (!Container) return;

	Container->RemoveChildAbility(AbilityID);
}

void AWOGPlayerController::SetTODString(ETimeOfDay CurrentTOD, FString& StringMain, FString& StringSec)
{
	MatchHUD == nullptr ? Cast<AWOGMatchHUD>(GetHUD()) : MatchHUD;
	if (!MatchHUD || !MatchHUD->HUDWidget) return;

	switch (CurrentTOD)
	{
	case ETimeOfDay::TOD_Dusk1:
		StringMain = FString("Dusk of the first day");
		StringSec = bIsAttacker ? FString("Destroy the Village") : FString("Defend the Village!");
		MatchHUD->HUDWidget->GetObjectiveWidget()->SetObjectiveText(bIsAttacker ? EObjectiveText::EOT_DestroyTheVillage : EObjectiveText::EOT_DefendTheVillage);
		break;

	case ETimeOfDay::TOD_Dawn2:
		StringMain = FString("Dawn of the second day");
		StringSec = FString("Rest and prepare for the night!");
		MatchHUD->HUDWidget->GetObjectiveWidget()->SetObjectiveText(EObjectiveText::EOT_PrepareForTheNight);
		break;

	case ETimeOfDay::TOD_Dusk2:
		StringMain = FString("Dusk of the second day");
		StringSec = bIsAttacker ? FString("Destroy the Village") : FString("Defend the Village!");
		MatchHUD->HUDWidget->GetObjectiveWidget()->SetObjectiveText(bIsAttacker ? EObjectiveText::EOT_DestroyTheVillage : EObjectiveText::EOT_DefendTheVillage);
		break;

	case ETimeOfDay::TOD_Dawn3:
		StringMain = FString("Dawn of the final day");
		StringSec = FString("Rest and prepare for the night!");
		MatchHUD->HUDWidget->GetObjectiveWidget()->SetObjectiveText(EObjectiveText::EOT_PrepareForTheNight);
		break;

	case ETimeOfDay::TOD_Dusk3:
		StringMain = FString("Dusk of the final day");
		StringSec = bIsAttacker ? FString("Destroy the Village") : FString("Defend the Village!");
		MatchHUD->HUDWidget->GetObjectiveWidget()->SetObjectiveText(bIsAttacker ? EObjectiveText::EOT_DestroyTheVillage : EObjectiveText::EOT_DefendTheVillage);
		break;

	case ETimeOfDay::TOD_Dawn4:
		StringMain = FString("Game Over!");
		StringSec = FString("");
		MatchHUD->HUDWidget->GetObjectiveWidget()->RemoveFromParent();
		break;

	default:
		return;
	}
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
}

void AWOGPlayerController::Test_CreateWarningWidget(const FString& Attribute)
{
	//Init UIManagerSubsystem and variables
	TObjectPtr<UWOGUIManagerSubsystem> UIManager = ULocalPlayer::GetSubsystem<UWOGUIManagerSubsystem>(GetLocalPlayer());
	if (UIManager)
	{
		UIManager->CreateWarningWidget(Attribute);
		UE_LOG(WOGLogUI, Display, TEXT("CreateWarningWidget() called from WOGPlayerController class"));
	}
}

void AWOGPlayerController::Client_CreateEndgameWidget_Implementation()
{
	MatchHUD == nullptr ? Cast<AWOGMatchHUD>(GetHUD()) : MatchHUD;
	if (!MatchHUD) return;

	MatchHUD->AddEndgameWidget();	
}

void AWOGPlayerController::Client_ResetHUD_Implementation()
{
	MatchHUD == nullptr ? Cast<AWOGMatchHUD>(GetHUD()) : MatchHUD;
	if (MatchHUD)
	{
		MatchHUD->ResetHUDAfterRespawn();
	}
}

void AWOGPlayerController::CreateWarningWidget(const FString& Attribute)
{
	MatchHUD == nullptr ? Cast<AWOGMatchHUD>(GetHUD()) : MatchHUD;
	if (!MatchHUD || !MatchHUD->HUDWidget || !IsValid(MatchHUD->AttributeWarningClass)) return;

	TObjectPtr<UWOGWarningWidget> WarningWidget = Cast<UWOGWarningWidget>(CreateWidget<UUserWidget>(this, MatchHUD->AttributeWarningClass));
	if (WarningWidget)
	{
		WarningWidget->SetWarningText(Attribute);

		if (MatchHUD->HUDWidget->GetWarningBox())
		{
			MatchHUD->HUDWidget->GetWarningBox()->ClearChildren();
			MatchHUD->HUDWidget->GetWarningBox()->AddChild(WarningWidget);
		}
	}
}

void AWOGPlayerController::CreateGenericWarningWidget(const FString& Attribute)
{
	MatchHUD == nullptr ? Cast<AWOGMatchHUD>(GetHUD()) : MatchHUD;
	if (!MatchHUD || !MatchHUD->HUDWidget || !IsValid(MatchHUD->GenericWarningClass)) return;

	TObjectPtr<UWOGWarningWidget> WarningWidget = Cast<UWOGWarningWidget>(CreateWidget<UUserWidget>(this, MatchHUD->GenericWarningClass));
	if (WarningWidget)
	{
		WarningWidget->SetWarningText(Attribute);

		if (MatchHUD->HUDWidget->GetWarningBox())
		{
			MatchHUD->HUDWidget->GetWarningBox()->ClearChildren();
			MatchHUD->HUDWidget->GetWarningBox()->AddChild(WarningWidget);
		}
	}
}

void AWOGPlayerController::AddStaminaWidget()
{
	MatchHUD == nullptr ? Cast<AWOGMatchHUD>(GetHUD()) : MatchHUD;
	if (!MatchHUD || !IsValid(MatchHUD->StaminaBarClass)) return;

	TObjectPtr<AWOGBaseCharacter> BaseCharacter = Cast<AWOGBaseCharacter>(GetPawn());
	if (!BaseCharacter || !BaseCharacter->GetStaminaWidgetContainer() || !BaseCharacter->GetStaminaWidgetContainer()->GetContainer()) return;
	if (BaseCharacter->GetStaminaWidgetContainer()->GetContainer()->HasAnyChildren()) return;

	TObjectPtr<UWOGRoundProgressBar> StaminaBar = Cast<UWOGRoundProgressBar>(CreateWidget<UUserWidget>(this, MatchHUD->StaminaBarClass));
	if (StaminaBar)
	{
		BaseCharacter->GetStaminaWidgetContainer()->GetContainer()->AddChild(StaminaBar);
	}
}

void AWOGPlayerController::AddHoldProgressBar()
{
	MatchHUD == nullptr ? Cast<AWOGMatchHUD>(GetHUD()) : MatchHUD;
	if (!MatchHUD || !MatchHUD->HUDWidget || !IsValid(MatchHUD->HoldProgressBarWidgetClass)) return;

	HoldProgressBarWidget = Cast<UWOGHoldProgressBar>(CreateWidget<UUserWidget>(this, MatchHUD->HoldProgressBarWidgetClass));
	if (HoldProgressBarWidget && MatchHUD->HUDWidget->GetHoldBarContainer())
	{
		MatchHUD->HUDWidget->GetHoldBarContainer()->AddChild(HoldProgressBarWidget);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid HoldProgressBarWidget"));
	}
}

void AWOGPlayerController::RemoveHoldProgressBar()
{
	if (HoldProgressBarWidget)
	{
		HoldProgressBarWidget->RemoveFromParent();
	}
}

void AWOGPlayerController::AddRavenMarkerWidget(const int32& Amount)
{
	MatchHUD == nullptr ? Cast<AWOGMatchHUD>(GetHUD()) : MatchHUD;
	if (!MatchHUD || !MatchHUD->HUDWidget || !IsValid(MatchHUD->RavenMarkerWidgetClass)) return;

	RavenMarkerWidget = Cast<UWOGRavenMarkerWidget>(CreateWidget<UUserWidget>(this, MatchHUD->RavenMarkerWidgetClass));
	if (RavenMarkerWidget && MatchHUD->HUDWidget->GetHoldBarContainer())
	{
		MatchHUD->HUDWidget->GetHoldBarContainer()->AddChild(RavenMarkerWidget);
		RavenMarkerWidget->SetAmountAvailableMarkers(Amount);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid RavenMarkerWidget"));
	}
}

void AWOGPlayerController::RemoveRavenMarkerWidget()
{
	if (RavenMarkerWidget)
	{
		RavenMarkerWidget->RemoveFromParent();
	}
}

void AWOGPlayerController::AddScreenDamageWidget(const int32& DamageThreshold)
{
	MatchHUD == nullptr ? Cast<AWOGMatchHUD>(GetHUD()) : MatchHUD;
	if (!MatchHUD || !MatchHUD->HUDWidget || !IsValid(MatchHUD->ScreenDamageWidgetClass)) return;

	TObjectPtr<UWOGScreenDamage> ScreenDamageWidget = Cast<UWOGScreenDamage>(CreateWidget<UUserWidget>(this, MatchHUD->ScreenDamageWidgetClass));
	if (ScreenDamageWidget)
	{
		ScreenDamageWidget->SetRadiusValue(DamageThreshold);
		ScreenDamageWidget->AddToViewport();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid ScreenDamageWidget"));
	}
}
