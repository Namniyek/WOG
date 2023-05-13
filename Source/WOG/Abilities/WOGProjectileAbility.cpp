// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGProjectileAbility.h"
#include "WOG/PlayerCharacter/BasePlayerCharacter.h"
#include "WOG/PlayerController/WOGPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "WOG/Data/WOGAbilityDataAsset.h"


AWOGProjectileAbility::AWOGProjectileAbility()
{
	StartLocation = FVector();
	EndLocation = FVector();

	PrimaryActorTick.bCanEverTick = false;
}

void AWOGProjectileAbility::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWOGProjectileAbility, EndLocation);
}

void AWOGProjectileAbility::BeginPlay()
{
	Super::BeginPlay();

	Init();

	OwnerCharacter = OwnerCharacter == nullptr ? Cast<ABasePlayerCharacter>(GetOwner()) : OwnerCharacter;
	if (OwnerCharacter && OwnerCharacter->GetMesh())
	{
		IdleSystemLeft = UNiagaraFunctionLibrary::SpawnSystemAttached(IdleParticleSystem, OwnerCharacter->GetMesh(), FName("Hand_L_Magic"), FVector(), FRotator(), EAttachLocation::SnapToTargetIncludingScale, false);
		IdleSystemRight = UNiagaraFunctionLibrary::SpawnSystemAttached(IdleParticleSystem, OwnerCharacter->GetMesh(), FName("Hand_R_Magic"), FVector(), FRotator(), EAttachLocation::SnapToTargetIncludingScale, false);
	}
}

void AWOGProjectileAbility::Equip()
{
	OwnerCharacter = OwnerCharacter == nullptr ? Cast<ABasePlayerCharacter>(GetOwner()) : OwnerCharacter;

	Client_AddCrosshair();

	if (HasAuthority())
	{
		CosmeticEquip();
	}
}

void AWOGProjectileAbility::Unequip()
{
	Client_RemoveCrosshair();

	if (HasAuthority())
	{
		CosmeticUnequip();
	}

	AWOGBaseAbility::Unequip();
}

void AWOGProjectileAbility::Use()
{
	Super::Use();

	if (ProjectileClass && OwnerCharacter)
	{
		StartLocation = OwnerCharacter->GetActorLocation() + (OwnerCharacter->GetActorForwardVector() * 25);

		if (OwnerCharacter->GetCurrentTarget())
		{
			EndLocation = OwnerCharacter->GetCurrentTarget()->GetActorLocation();
		}
		else
		{
			EndLocation = CalculateEndVector();
		}

		FRotator StartRotation = (EndLocation - StartLocation).GetSafeNormal().Rotation();

		FTransform SpawnTransform;
		SpawnTransform.SetRotation(FQuat::MakeFromRotator(StartRotation));
		SpawnTransform.SetLocation(StartLocation);

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = OwnerCharacter;
		GetWorld()->SpawnActor<AActor>(ProjectileClass, SpawnTransform, SpawnParams);
	}
}

void AWOGProjectileAbility::Init()
{
	Super::Init();

	if (!AbilityDataAsset)
	{
		UE_LOG(LogTemp, Error, TEXT("No Valid Data Asset"));
		return;
	}

	CrosshairWidgetClass = AbilityDataAsset->CrosshairWidgetClass;
	ProjectileClass = AbilityDataAsset->ProjectileClass;
	ImpactSound = AbilityDataAsset->ImpactSound;
	ImpactParticleSystem = AbilityDataAsset->ImpactParticleSystem;
}

FVector AWOGProjectileAbility::CalculateEndVector()
{
	FHitResult TraceHitResult;
	GetBeamEndLocation(StartLocation, TraceHitResult);
	return TraceHitResult.Location;
}

void AWOGProjectileAbility::CosmeticUse()
{
	if (!OwnerCharacter) return;

	UAnimInstance* CharacterAnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
	if (CharacterAnimInstance && UseMontage)
	{
		FString SectionName = FString::FromInt(FMath::RandRange(0, 3));
		CharacterAnimInstance->Montage_Play(UseMontage, 1.f);
		CharacterAnimInstance->Montage_JumpToSection(FName(*SectionName));
	}

	FVector TempEndLocation;
	if (OwnerCharacter->GetCurrentTarget())
	{
		TempEndLocation = OwnerCharacter->GetCurrentTarget()->GetActorLocation();
		UE_LOG(LogTemp, Warning, TEXT("SetActorRotation() to Target"));
	}
	else
	{
		TempEndLocation = CalculateEndVector();
		UE_LOG(LogTemp, Warning, TEXT("SetActorRotation() to Trace"));
	}
	OwnerCharacter->SetActorRotation((EndLocation - OwnerCharacter->GetActorLocation()).GetSafeNormal2D().Rotation());

}

void AWOGProjectileAbility::CosmeticEquip()
{

}

void AWOGProjectileAbility::CosmeticUnequip()
{
	if (IdleSystemLeft)
	{
		IdleSystemLeft->Deactivate();
		IdleSystemLeft->DestroyComponent();
	}
	if (IdleSystemRight)
	{
		IdleSystemRight->Deactivate();
		IdleSystemRight->DestroyComponent();
	}
}

bool AWOGProjectileAbility::TraceUnderCrosshairs(FHitResult& OutHitResult, FVector& OutHitLocation)
{
	if (OwnerCharacter->GetFollowCamera() && OwnerCharacter->GetCameraBoom())
	{
		//Trace from camera world location outward
		const FVector Start(OwnerCharacter->GetFollowCamera()->GetComponentLocation() + OwnerCharacter->GetFollowCamera()->GetForwardVector() * (OwnerCharacter->GetCameraBoom()->TargetArmLength));
		const FVector End(Start + OwnerCharacter->GetFollowCamera()->GetForwardVector() * 15'000.f);
		OutHitLocation = End;
		GetWorld()->LineTraceSingleByChannel(OutHitResult, Start, End, ECollisionChannel::ECC_Visibility);

		if (OutHitResult.bBlockingHit)
		{
			OutHitLocation = OutHitResult.Location;
			return true;
		}
	}
	return false;
}

void AWOGProjectileAbility::GetBeamEndLocation(const FVector& TraceStartLocation, FHitResult& OutHitResult)
{
	//Check for crosshairs trace hit
	FHitResult CrosshairHitResult;
	FVector OutBeamLocation;
	bool bCrosshairHit(TraceUnderCrosshairs(CrosshairHitResult, OutBeamLocation));

	if (bCrosshairHit)
	{
		//Tentative beam location
		OutBeamLocation = CrosshairHitResult.Location;
	}
	else //no crosshair trace hit
	{
		//OutBeamLocaton is end location of line trace
	}

	//Perform a second trace from character
	const FVector TraceStart = TraceStartLocation;
	const FVector StartToEnd = OutBeamLocation - TraceStartLocation;
	const FVector TraceEnd = OutBeamLocation + StartToEnd * 1.25f;

	GetWorld()->LineTraceSingleByChannel(OutHitResult, TraceStart, TraceEnd, ECollisionChannel::ECC_Visibility);

	//object between character and beam end point
	if (!OutHitResult.bBlockingHit)
	{
		OutHitResult.Location = OutBeamLocation;
	}
}

void AWOGProjectileAbility::Client_AddCrosshair_Implementation()
{
	if (CrosshairWidgetClass)
	{
		CrosshairWidget = CreateWidget(GetWorld(), CrosshairWidgetClass);
	}

	if (CrosshairWidget)
	{
		CrosshairWidget->AddToViewport();
	}
}

void AWOGProjectileAbility::Client_RemoveCrosshair_Implementation()
{
	if (CrosshairWidget)
	{
		CrosshairWidget->RemoveFromParent();
	}
}
