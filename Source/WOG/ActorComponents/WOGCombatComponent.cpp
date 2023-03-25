// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGCombatComponent.h"
#include "Net/UnrealNetwork.h"
#include "WOG/PlayerCharacter/BasePlayerCharacter.h"
#include "WOG/Weapons/WOGBaseWeapon.h"
#include "DidItHitActorComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

UWOGCombatComponent::UWOGCombatComponent()
{
	SetIsReplicated(true);
}

void UWOGCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UWOGCombatComponent, MainWeapon);
	DOREPLIFETIME(UWOGCombatComponent, SecondaryWeapon);
	DOREPLIFETIME(UWOGCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UWOGCombatComponent, EquippedWeaponType);
}

void UWOGCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	OwnerPlayerCharacter = OwnerPlayerCharacter == nullptr ? Cast<ABasePlayerCharacter>(GetOwner()) : OwnerPlayerCharacter;
	/*if (OwnerPlayerCharacter->IsLocallyControlled() && DefaultWeaponClass)
	{
		Server_CreateMainWeapon(DefaultWeaponClass);
	}*/
}

void UWOGCombatComponent::Server_CreateMainWeapon_Implementation(TSubclassOf<AWOGBaseWeapon> WeaponToCreate)
{
	CreateMainWeapon(WeaponToCreate);
}

void UWOGCombatComponent::CreateMainWeapon(TSubclassOf<AWOGBaseWeapon> WeaponToCreate)
{
	OwnerPlayerCharacter = OwnerPlayerCharacter == nullptr ? Cast<ABasePlayerCharacter>(GetOwner()) : OwnerPlayerCharacter;
	if (!OwnerPlayerCharacter) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = OwnerPlayerCharacter;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	MainWeapon = GetWorld()->SpawnActor<AWOGBaseWeapon>(WeaponToCreate, OwnerPlayerCharacter->GetActorTransform(), SpawnParams);
	
	if (MainWeapon)
	{
		MainWeapon->SetOwner(OwnerPlayerCharacter);
		MainWeapon->OwnerCharacter = OwnerPlayerCharacter;
		MainWeapon->Server_SetWeaponState(EWeaponState::EWS_Stored);
		MainWeapon->InitTraceComponent();

		if (MainWeapon->GetTraceComponent() && SecondaryWeapon)
		{
			MainWeapon->GetTraceComponent()->MyActorsToIgnore.AddUnique(SecondaryWeapon);
		}
	}

}

void UWOGCombatComponent::OnRep_MainWeapon()
{
	MainWeapon->AttachToBack();
}

void UWOGCombatComponent::Server_CreateSecondaryWeapon_Implementation(TSubclassOf<AWOGBaseWeapon> WeaponToCreate)
{
	CreateSecondaryWeapon(WeaponToCreate);
}

void UWOGCombatComponent::CreateSecondaryWeapon(TSubclassOf<AWOGBaseWeapon> WeaponToCreate)
{
	OwnerPlayerCharacter = OwnerPlayerCharacter == nullptr ? Cast<ABasePlayerCharacter>(GetOwner()) : OwnerPlayerCharacter;
	if (!OwnerPlayerCharacter) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = OwnerPlayerCharacter;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	SecondaryWeapon = GetWorld()->SpawnActor<AWOGBaseWeapon>(WeaponToCreate, OwnerPlayerCharacter->GetActorTransform(), SpawnParams);

	if (SecondaryWeapon)
	{
		SecondaryWeapon->SetOwner(OwnerPlayerCharacter);
		SecondaryWeapon->OwnerCharacter = OwnerPlayerCharacter;
		SecondaryWeapon->Server_SetWeaponState(EWeaponState::EWS_Stored);
		SecondaryWeapon->InitTraceComponent();

		if (SecondaryWeapon->GetTraceComponent() && MainWeapon)
		{
			SecondaryWeapon->GetTraceComponent()->MyActorsToIgnore.AddUnique(MainWeapon);
		}
	}

}

void UWOGCombatComponent::OnRep_SecWeapon()
{
	SecondaryWeapon->AttachToBack();
}

void UWOGCombatComponent::SetEquippedWeapon(AWOGBaseWeapon* NewEquippedWeapon)
{
	EquippedWeapon = NewEquippedWeapon;
	if (EquippedWeapon)
	{
		EquippedWeaponType = EquippedWeapon->GetWeaponType();
	}
	if (EquippedWeapon == nullptr)
	{
		EquippedWeaponType = EWeaponType::EWT_Unarmed;
	}
}

void UWOGCombatComponent::EquipMainWeapon()
{

	if (MainWeapon)
	{
		MainWeapon->Server_Equip();
	}
}

void UWOGCombatComponent::EquipSecondaryWeapon()
{

	if (SecondaryWeapon)
	{
		SecondaryWeapon->Server_Equip();
	}
}

void UWOGCombatComponent::UnequipMainWeapon()
{

	if (MainWeapon)
	{
		MainWeapon->Server_Unequip();
	}
}

void UWOGCombatComponent::UnequipSecondaryWeapon()
{

	if (SecondaryWeapon)
	{
		SecondaryWeapon->Server_Unequip();
	}
}

void UWOGCombatComponent::SwapWeapons()
{

	if (EquippedWeapon == MainWeapon)
	{
		MainWeapon->Server_Swap();
		SecondaryWeapon->Server_Equip();
	}
	else if (EquippedWeapon == SecondaryWeapon)
	{
		SecondaryWeapon->Server_Swap();
		MainWeapon->Server_Equip();
	}
	if (OwnerPlayerCharacter)
	{
		OwnerPlayerCharacter->Server_SetCharacterState(ECharacterState::ECS_Unnoccupied);
	}
}

void UWOGCombatComponent::DropWeapons()
{
}

void UWOGCombatComponent::AttackLight()
{
	OwnerPlayerCharacter = OwnerPlayerCharacter == nullptr ? Cast<ABasePlayerCharacter>(GetOwner()) : OwnerPlayerCharacter;
	if (!EquippedWeapon || !OwnerPlayerCharacter) return;

	if (OwnerPlayerCharacter->CharacterState != ECharacterState::ECS_Attacking || EquippedWeapon->GetIsInCombo())
	{
		EquippedWeapon->Server_AttackLight();
		OwnerPlayerCharacter->Server_SetCharacterState(ECharacterState::ECS_Attacking);
	}
}

void UWOGCombatComponent::AttackHeavy()
{
	OwnerPlayerCharacter = OwnerPlayerCharacter == nullptr ? Cast<ABasePlayerCharacter>(GetOwner()) : OwnerPlayerCharacter;
	if (!EquippedWeapon || !OwnerPlayerCharacter) return;
	OwnerPlayerCharacter->Server_SetCharacterState(ECharacterState::ECS_Attacking);

	EquippedWeapon->Server_AttackHeavy();
}

void UWOGCombatComponent::Block()
{
	OwnerPlayerCharacter = OwnerPlayerCharacter == nullptr ? Cast<ABasePlayerCharacter>(GetOwner()) : OwnerPlayerCharacter;
	if (!OwnerPlayerCharacter) return;
	if (OwnerPlayerCharacter->GetCharacterState() == ECharacterState::ECS_Unnoccupied)
	{
		OwnerPlayerCharacter->Server_SetCharacterState(ECharacterState::ECS_Blocking);
	}

	if (!EquippedWeapon)
	{
		return;
	}
	EquippedWeapon->Server_Block();
}

void UWOGCombatComponent::StopBlocking()
{
	OwnerPlayerCharacter = OwnerPlayerCharacter == nullptr ? Cast<ABasePlayerCharacter>(GetOwner()) : OwnerPlayerCharacter;
	if (!OwnerPlayerCharacter) return;

	if (OwnerPlayerCharacter->GetCharacterState() == ECharacterState::ECS_Blocking)
	{
		OwnerPlayerCharacter->Server_SetCharacterState(ECharacterState::ECS_Unnoccupied);
	}

	if (!EquippedWeapon)
	{
		return;
	}
	EquippedWeapon->Server_StopBlocking();

}

void UWOGCombatComponent::Multicast_HandleCosmeticHit_Implementation(const ECosmeticHit& HitType, const FHitResult& Hit, const FVector& WeaponLocation, const AWOGBaseWeapon* InstigatorWeapon)
{
	switch (HitType)
	{
	case ECosmeticHit::ECH_BodyHit:
		HandleCosmeticBodyHit(Hit, WeaponLocation, InstigatorWeapon);
		break;
	case ECosmeticHit::ECH_BlockingWeapon:
		HandleCosmeticBlock(InstigatorWeapon);
		break;
	case ECosmeticHit::ECH_AttackingWeapon:
		HandleCosmeticWeaponClash();
		break;
	}
}

void UWOGCombatComponent::HandleCosmeticBodyHit(const FHitResult& Hit, const FVector& WeaponLocation, const AWOGBaseWeapon* InstigatorWeapon)
{
	FName HitDirection = CalculateHitDirection(Hit, WeaponLocation);
	PlayHitReactMontage(HitDirection);

	if (InstigatorWeapon)
	{
		UGameplayStatics::PlaySoundAtLocation(this, InstigatorWeapon->GetHitSound(), OwnerPlayerCharacter->GetActorLocation());
	}
}

FName UWOGCombatComponent::CalculateHitDirection(const FHitResult& Hit, const FVector& WeaponLocation)
{
	// Get the location of the impact point
	FVector ImpactPoint = Hit.Location;

	// Calculate the attack direction
	FVector AttackDirection = (ImpactPoint - WeaponLocation).GetSafeNormal();

	// Calculate the hit direction
	FVector CharacterForward = OwnerPlayerCharacter->GetActorForwardVector();
	LastHitDirection = FVector::CrossProduct(AttackDirection, CharacterForward);

	// Determine the hit direction
	if (FMath::Abs(LastHitDirection.Z) > FMath::Abs(LastHitDirection.X) && FMath::Abs(LastHitDirection.Z) > FMath::Abs(LastHitDirection.Y))
	{
		// Hit came from the front or back
		if (FVector::DotProduct(AttackDirection, CharacterForward) > 0.0f)
		{
			// Hit came from the front
			return FName("Front");
		}
		else
		{
			// Hit came from the back
			return FName("Back");
		}
	}
	else
	{
		// Hit came from the left or right
		if (LastHitDirection.Z > 0.0f)
		{
			// Hit came from the right
			return FName("Right");
		}
		else
		{
			// Hit came from the left
			return FName("Left");
		}
	}
}

void UWOGCombatComponent::PlayHitReactMontage(FName Section)
{
	if (!OwnerPlayerCharacter) return;

	UAnimInstance* CharacterAnimInstance = OwnerPlayerCharacter->GetMesh()->GetAnimInstance();
	if (!CharacterAnimInstance) return;

	if (EquippedWeapon && EquippedWeapon->GetHurtMontage())
	{
		CharacterAnimInstance->Montage_Play(EquippedWeapon->GetHurtMontage(), 1.f);
		CharacterAnimInstance->Montage_JumpToSection(Section);
	}
}

void UWOGCombatComponent::HandleCosmeticBlock(const AWOGBaseWeapon* InstigatorWeapon)
{
	if (!OwnerPlayerCharacter || !EquippedWeapon) return;

	if (EquippedWeapon->GetBlockSound())
	{
		UGameplayStatics::PlaySoundAtLocation(this, EquippedWeapon->GetBlockSound(), OwnerPlayerCharacter->GetActorLocation());
	}

	if (EquippedWeapon->GetBlockMontage())
	{
		UAnimInstance* CharacterAnimInstance = OwnerPlayerCharacter->GetMesh()->GetAnimInstance();
		if (!CharacterAnimInstance) return;

		if (CharacterAnimInstance)
		{
			CharacterAnimInstance->Montage_Play(EquippedWeapon->GetBlockMontage(), 1.f);
			CharacterAnimInstance->Montage_JumpToSection(FName("Impact"));
		}
	}
}

void UWOGCombatComponent::HandleCosmeticWeaponClash()
{
	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Blue, FString("Weapons Clashed!"));
}

//void UWOGCombatComponent::Multicast_HandleCosmeticBlock_Implementation(const AWOGBaseWeapon* InstigatorWeapon)
//{
//
//}
