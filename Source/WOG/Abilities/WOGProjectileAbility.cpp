// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGProjectileAbility.h"
#include "WOG/PlayerCharacter/BasePlayerCharacter.h"
#include "Blueprint/UserWidget.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"


AWOGProjectileAbility::AWOGProjectileAbility()
{

}

void AWOGProjectileAbility::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = OwnerCharacter == nullptr ? (TObjectPtr<ABasePlayerCharacter>) Cast<ABasePlayerCharacter>(GetOwner()) : OwnerCharacter;
	UE_LOG(LogTemp, Warning, TEXT("OwnerCharacter is valid: %d"), IsValid(OwnerCharacter));
}

void AWOGProjectileAbility::Equip()
{
	OwnerCharacter = OwnerCharacter == nullptr ? (TObjectPtr<ABasePlayerCharacter>) Cast<ABasePlayerCharacter>(GetOwner()) : OwnerCharacter;

	Client_AddCrosshair();

	if (HasAuthority())
	{
		HandleCosmeticEquip();
		Multicast_HandleCosmeticEquip();
	}
}

void AWOGProjectileAbility::Unequip()
{
	Client_RemoveCrosshair();

	if (HasAuthority())
	{
		HandleCosmeticUnequip();
		Multicast_HandleCosmeticUnequip();
	}

	AWOGBaseAbility::Unequip();
}

void AWOGProjectileAbility::Use()
{
	Super::Use();

	if (ProjectileClass && OwnerCharacter)
	{
		FVector StartLocation = OwnerCharacter->GetActorLocation() + (OwnerCharacter->GetActorForwardVector() * 50);
		FVector EndLocation;
		if (OwnerCharacter->GetCurrentTarget())
		{
			EndLocation = OwnerCharacter->GetCurrentTarget()->GetActorLocation();
		}
		else
		{
			FHitResult TraceHitResult;

			GetBeamEndLocation(StartLocation, TraceHitResult);
			EndLocation = TraceHitResult.Location;
		}

		FRotator StartRotation = (EndLocation - StartLocation).GetSafeNormal().Rotation();

		FTransform SpawnTransform;
		SpawnTransform.SetRotation(FQuat::MakeFromRotator(StartRotation));
		SpawnTransform.SetLocation(StartLocation);

		OwnerCharacter->SetActorRotation((EndLocation - StartLocation).GetSafeNormal2D().Rotation());
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = OwnerCharacter;
		GetWorld()->SpawnActor<AActor>(ProjectileClass, SpawnTransform, SpawnParams);
	}
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
}

void AWOGProjectileAbility::Multicast_HandleCosmeticEquip_Implementation()
{
	if (HasAuthority()) return;
	HandleCosmeticEquip();
}

void AWOGProjectileAbility::HandleCosmeticEquip()
{
	if (OwnerCharacter && OwnerCharacter->GetMesh())
	{
		IdleSystemLeft = UNiagaraFunctionLibrary::SpawnSystemAttached(IdleParticleSystem, OwnerCharacter->GetMesh(), FName("Hand_L_Magic"), FVector(), FRotator(), EAttachLocation::SnapToTargetIncludingScale, false);
		IdleSystemRight = UNiagaraFunctionLibrary::SpawnSystemAttached(IdleParticleSystem, OwnerCharacter->GetMesh(), FName("Hand_R_Magic"), FVector(), FRotator(), EAttachLocation::SnapToTargetIncludingScale, false);
	}
}

void AWOGProjectileAbility::Multicast_HandleCosmeticUnequip_Implementation()
{
	if (HasAuthority()) return;
	HandleCosmeticUnequip();
}

void AWOGProjectileAbility::HandleCosmeticUnequip()
{
	if (IdleSystemLeft)
	{
		IdleSystemLeft->DestroyComponent();
	}
	if (IdleSystemRight)
	{
		IdleSystemRight->DestroyComponent();
	}
}

bool AWOGProjectileAbility::TraceUnderCrosshairs(FHitResult& OutHitResult, FVector& OutHitLocation)
{
	//Get Viewport size
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	//Get screen space location of the crosshair
	FVector2D CrosshairLocation((ViewportSize.X / 2.f), (ViewportSize.Y / 2.f));
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;

	//Get world position and direction of the crosshair
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(this, 0), CrosshairLocation, CrosshairWorldPosition, CrosshairWorldDirection);

	if (bScreenToWorld)
	{
		//Trace from Crosshair world location outward
		const FVector Start(CrosshairWorldPosition);
		const FVector End(Start + CrosshairWorldDirection * 50'000.f);
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

void AWOGProjectileAbility::GetBeamEndLocation(const FVector& StartLocation, FHitResult& OutHitResult)
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

	//Perform a second trace from character barrel
	const FVector TraceStart = StartLocation;
	const FVector StartToEnd = OutBeamLocation - StartLocation;
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
