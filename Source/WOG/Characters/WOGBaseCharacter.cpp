// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGBaseCharacter.h"
#include "Net/UnrealNetwork.h"
#include "WOG/ActorComponents/WOGAttributesComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#


AWOGBaseCharacter::AWOGBaseCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	Attributes = CreateDefaultSubobject<UWOGAttributesComponent>(TEXT("AttributesComponent"));
	Attributes->SetIsReplicated(true);

	SpeedRequiredForLeap = 750.f;
}

void AWOGBaseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWOGBaseCharacter, CharacterState);
}

void AWOGBaseCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &ThisClass::ReceiveDamage);
	}
}

void AWOGBaseCharacter::Server_SetCharacterState_Implementation(ECharacterState NewState, AController* InstigatedBy)
{
	Multicast_SetCharacterState(NewState, InstigatedBy);
	SetCharacterState(NewState, InstigatedBy);
}

void AWOGBaseCharacter::Multicast_SetCharacterState_Implementation(ECharacterState NewState, AController* InstigatedBy)
{
	if (!HasAuthority())
	{
		SetCharacterState(NewState, InstigatedBy);
	}
}

void AWOGBaseCharacter::SetCharacterState(ECharacterState NewState, AController* InstigatedBy)
{
	CharacterState = NewState;

	switch (CharacterState)
	{
	case ECharacterState::ECS_Elimmed:
		HandleStateElimmed(InstigatedBy);
		break;
	case ECharacterState::ECS_Sprinting:
		HandleStateSprinting();
		break;
	case ECharacterState::ECS_Dodging :
		HandleStateDodging();
		break;
	case ECharacterState::ECS_Unnoccupied:
		HandleStateUnnoccupied();
		break;
	case ECharacterState::ECS_Attacking:
		HandleStateAttacking();
		break;
	}
}

void AWOGBaseCharacter::HandleStateElimmed(AController* InstigatedBy)
{
	//To be overriden in Children
}

void AWOGBaseCharacter::HandleStateSprinting()
{
	//To be overriden in Children
}

void AWOGBaseCharacter::HandleStateUnnoccupied()
{
	//To be overriden in Children
}

void AWOGBaseCharacter::HandleStateDodging()
{
	//To be overriden in Children
}

void AWOGBaseCharacter::HandleStateAttacking()
{
	//To be overriden in Children
}

void AWOGBaseCharacter::BroadcastHit_Implementation(AActor* AgressorActor, const FHitResult& Hit, const float& DamageToApply, AActor* InstigatorWeapon)
{

}

bool AWOGBaseCharacter::IsHitFrontal(const float& AngleTolerance, const AActor* Victim, const AActor* Agressor)
{
	FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(Victim->GetActorLocation(), Agressor->GetActorLocation());
	FRotator DeltaRotator = UKismetMathLibrary::NormalizedDeltaRotator(GetActorRotation(), LookAtRotation);

	bool bIsHitFrontal = 
		!(UKismetMathLibrary::InRange_FloatFloat(DeltaRotator.Yaw, -180, -45) ||
		UKismetMathLibrary::InRange_FloatFloat(DeltaRotator.Yaw, 45, 180));

	return bIsHitFrontal;
}

void AWOGBaseCharacter::Multicast_HandleCosmeticHit_Implementation(const ECosmeticHit& HitType, const FHitResult& Hit, const FVector& WeaponLocation, const AWOGBaseWeapon* InstigatorWeapon)
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

void AWOGBaseCharacter::HandleCosmeticBodyHit(const FHitResult& Hit, const FVector& WeaponLocation, const AWOGBaseWeapon* InstigatorWeapon)
{
	//To be overriden in Children
}

FName AWOGBaseCharacter::CalculateHitDirection(const FHitResult& Hit, const FVector& WeaponLocation)
{
	// Get the location of the impact point
	FVector ImpactPoint = Hit.Location;

	// Calculate the attack direction
	FVector AttackDirection = (ImpactPoint - WeaponLocation).GetSafeNormal();

	// Calculate the hit direction
	FVector CharacterForward = GetActorForwardVector();
	LastHitDirection = FVector::CrossProduct(AttackDirection, CharacterForward);

	// Determine the hit direction
	if (FMath::Abs(LastHitDirection.Z) > FMath::Abs(LastHitDirection.X) && FMath::Abs(LastHitDirection.Z) > FMath::Abs(LastHitDirection.Y))
	{
		// Hit came from the front or back
		if (FVector::DotProduct(AttackDirection, CharacterForward) > 0.0f)
		{
			// Hit came from the front
			UE_LOG(LogTemp, Warning, TEXT("FRONT"));
			return FName("Front");
		}
		else
		{
			// Hit came from the back
			UE_LOG(LogTemp, Warning, TEXT("BACK"));
			return FName("Back");
		}
	}
	else
	{
		// Hit came from the left or right
		if (LastHitDirection.Z < 0.0f)
		{
			// Hit came from the right
			UE_LOG(LogTemp, Warning, TEXT("RIGHT"));
			return FName("Right");
		}
		else
		{
			// Hit came from the left
			UE_LOG(LogTemp, Warning, TEXT("LEFT"));
			return FName("Left");
		}
	}
}

void AWOGBaseCharacter::PlayHitReactMontage(FName Section)
{
	//To be overriden in Children
}

void AWOGBaseCharacter::HandleCosmeticBlock(const AWOGBaseWeapon* InstigatorWeapon)
{
	//To be overriden in Children
}

void AWOGBaseCharacter::HandleCosmeticWeaponClash()
{
	//To be overriden in Children
}

void AWOGBaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWOGBaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AWOGBaseCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if (!Attributes) return;
	Attributes->Server_UpdateHealth(Damage, InstigatedBy);
}

void AWOGBaseCharacter::Elim(bool bPlayerLeftGame)
{
	//To be overriden in Children
}

