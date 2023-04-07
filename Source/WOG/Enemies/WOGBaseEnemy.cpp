// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGBaseEnemy.h"
#include "LockOnTargetComponent.h"
#include "TargetingHelperComponent.h"
#include "WOG/ActorComponents/WOGAttributesComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AIController.h"
#include "WOG/Weapons/WOGBaseWeapon.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AWOGBaseEnemy::AWOGBaseEnemy()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	LockOnTarget = CreateDefaultSubobject<ULockOnTargetComponent>(TEXT("LockOnTargetComponent"));
	LockOnTarget->SetIsReplicated(true);
	TargetAttractor = CreateDefaultSubobject<UTargetingHelperComponent>(TEXT("TargetAttractor"));
	TargetAttractor->SetIsReplicated(true);
}

void AWOGBaseEnemy::BeginPlay()
{
	Super::BeginPlay();
	
}

void AWOGBaseEnemy::BroadcastHit_Implementation(AActor* AgressorActor, const FHitResult& Hit, const float& DamageToApply, AActor* InstigatorWeapon)
{
	if (CharacterState == ECharacterState::ECS_Elimmed) return;

	TObjectPtr<AWOGBaseWeapon> Weapon = Cast<AWOGBaseWeapon>(InstigatorWeapon);
	Multicast_HandleCosmeticHit(ECosmeticHit::ECH_BodyHit, Hit, InstigatorWeapon->GetActorLocation(), Weapon);

	TObjectPtr<AWOGBaseCharacter> AgressorCharacter = Cast<AWOGBaseCharacter>(AgressorActor);
	if (AgressorCharacter)
	{
		UGameplayStatics::ApplyDamage(this, -DamageToApply, AgressorCharacter->GetController(), AgressorActor, UDamageType::StaticClass());
	}
}

void AWOGBaseEnemy::HandleStateElimmed(AController* InstigatedBy)
{
	Elim(false);
}

void AWOGBaseEnemy::Elim(bool bPlayerLeftGame)
{
	Multicast_Elim(bPlayerLeftGame);
}

void AWOGBaseEnemy::Multicast_Elim_Implementation(bool bPlayerLeftGame)
{
	GetMesh()->SetCollisionProfileName(FName("Ragdoll"));
	GetMesh()->SetAllBodiesSimulatePhysics(true);
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
	FVector ImpulseDirection = LastHitDirection.GetSafeNormal() * 45000.f;
	GetMesh()->AddImpulse(ImpulseDirection);
	LockOnTarget->ClearTargetManual();
	TargetAttractor->bCanBeCaptured = false;

	/*
	**Handle destroy timer
	*/
	GetWorld()->GetTimerManager().SetTimer(ElimTimer, this, &ThisClass::ElimTimerFinished, ElimDelay);
}

void AWOGBaseEnemy::ElimTimerFinished()
{
	Destroy();
}

void AWOGBaseEnemy::HandleCosmeticBodyHit(const FHitResult& Hit, const FVector& WeaponLocation, const AWOGBaseWeapon* InstigatorWeapon)
{
	FName HitDirection = CalculateHitDirection(Hit, WeaponLocation);
	PlayHitReactMontage(HitDirection);
}

void AWOGBaseEnemy::PlayHitReactMontage(FName Section)
{
	UAnimInstance* CharacterAnimInstance = GetMesh()->GetAnimInstance();
	if (!CharacterAnimInstance || !UnarmedHurtMontage) return;

	CharacterAnimInstance->Montage_Play(UnarmedHurtMontage, 1.f);
	CharacterAnimInstance->Montage_JumpToSection(Section);
}

void AWOGBaseEnemy::HandleCosmeticBlock(const AWOGBaseWeapon* InstigatorWeapon)
{
}

void AWOGBaseEnemy::HandleCosmeticWeaponClash()
{
}

void AWOGBaseEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

