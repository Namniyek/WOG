// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/WOGRangedWeaponBase.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/RotatingMovementComponent.h"
#include "WOG/PlayerCharacter/BasePlayerCharacter.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/AGR_CombatManager.h"
#include "Libraries/WOGBlueprintLibrary.h"

// Sets default values
AWOGRangedWeaponBase::AWOGRangedWeaponBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject <USceneComponent>(TEXT("RootComponent"));
	SetRootComponent(Root);

	Mesh = CreateDefaultSubobject <UStaticMeshComponent>(TEXT("RangedMesh"));
	Mesh->SetupAttachment(GetRootComponent());
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetIsReplicated(true);

	ThrowRotation = CreateDefaultSubobject <URotatingMovementComponent>(TEXT("ThrowRotationComponent"));
	ThrowRotation->SetIsReplicated(true);

	RecallRotation = CreateDefaultSubobject <URotatingMovementComponent>(TEXT("RecallRotationComponent"));
	RecallRotation->SetIsReplicated(true);

	ProjectileComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComponent"));
	ProjectileComponent->SetIsReplicated(true);

	CombatManager = CreateDefaultSubobject<UAGR_CombatManager>(TEXT("CombatManager"));
	CombatManager->SetIsReplicated(true);
	CombatManager->OnAttackHitEvent.AddDynamic(this, &ThisClass::OnAttackHit);
	CombatManager->StartSocketTraceName = FName("Trace_01");
	CombatManager->EndSocketTraceName = FName("Trace_02");


	RecallTime = 0.5f;
	TraceDelay = 0.005f;

	bHitFound = false;
}

void AWOGRangedWeaponBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	InitWeapon();
}

void AWOGRangedWeaponBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWOGRangedWeaponBase, OwnerCharacter);
}

// Called when the game starts or when spawned
void AWOGRangedWeaponBase::BeginPlay()
{
	Super::BeginPlay();

	if (CombatManager)
	{
		CombatManager->AddTraceMesh(Mesh, EAGR_CombatColliderType::ComplexBoxTrace);

		GetWorldTimerManager().SetTimer(TraceDelayTimer, this, &ThisClass::StartTrace, TraceDelay);
	}
}

void AWOGRangedWeaponBase::StartTrace()
{
	if (CombatManager)
	{
		CombatManager->StartTrace();
		UE_LOG(LogTemp, Warning, TEXT("Trace started after timer"));
	}
}

void AWOGRangedWeaponBase::InitWeapon()
{
	OwnerCharacter = GetOwner() ? Cast<ABasePlayerCharacter>(GetOwner()) : nullptr;

	const FString WeaponTablePath{ TEXT("Engine.DataTable'/Game/Data/Weapons/DT_Weapons.DT_Weapons'") };
	UDataTable* WeaponTableObject = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *WeaponTablePath));

	if (!WeaponTableObject) return;

	TArray<FName> WeaponNamesArray = WeaponTableObject->GetRowNames();
	FWeaponDataTable* WeaponDataRow = nullptr;

	for (auto WeaponRowName : WeaponNamesArray)
	{
		if (WeaponRowName == WeaponName)
		{
			WeaponDataRow = WeaponTableObject->FindRow<FWeaponDataTable>(WeaponName, TEXT(""));
			break;
		}
	}
	if (WeaponDataRow)
	{
		Mesh->SetStaticMesh(WeaponDataRow->MeshSecondary);
		WeaponData = *WeaponDataRow;
	}
}

void AWOGRangedWeaponBase::OnAttackHit(FHitResult Hit, UPrimitiveComponent* HitMesh)
{
	if (bHitFound) return;

	bHitFound = true;

	if (CombatManager)
	{
		CombatManager->EndTrace();
	}

	if (ProjectileComponent)
	{
		ProjectileComponent->StopMovementImmediately();
		ProjectileComponent->ProjectileGravityScale = 0.f;
	}

	if (ThrowRotation)
	{
		ThrowRotation->RotationRate = FRotator::ZeroRotator;
	}

	DropWeapon(Hit.ImpactNormal);

	if (Hit.GetActor() && OwnerCharacter)
	{
		//Check if we hit character
		TObjectPtr<IAttributesInterface> AttributesInterface = Cast<IAttributesInterface>(Hit.GetActor());
		if (AttributesInterface)
		{
			AttributesInterface->Execute_BroadcastHit(Hit.GetActor(), OwnerCharacter, Hit, WeaponData.BaseDamage, UWOGBlueprintLibrary::GetEquippedWeapon(OwnerCharacter));
		}
	}
}

void AWOGRangedWeaponBase::DropWeapon(const FVector& ImpactNormal)
{
	if (Mesh)
	{
		Mesh->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
		Mesh->SetSimulatePhysics(true);
		Mesh->AddImpulse((ImpactNormal * 100.f), NAME_None, true);
	}
}

// Called every frame
void AWOGRangedWeaponBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWOGRangedWeaponBase::RecallWeapon_Implementation()
{
	OwnerCharacter = OwnerCharacter == nullptr ? Cast<ABasePlayerCharacter>(GetOwner()) : OwnerCharacter;
	if (!OwnerCharacter) return;

	if (CombatManager)
	{
		CombatManager->EndTrace();
	}

	if (ThrowRotation)
	{
		ThrowRotation->RotationRate = FRotator::ZeroRotator;
	}

	if (RecallRotation)
	{
		RecallRotation->RotationRate = bHitFound ? FRotator(0.f, -100.f, 0.f) : FRotator(0.f, 100.f, 0.f);
	}
}

