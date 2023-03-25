// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WOGBaseActorComponent.h"
#include "WOGAttributesComponent.generated.h"

/**
 * 
 */

UENUM(BlueprintType)
enum class EAttributeType : uint8
{
	AT_Health UMETA(DisplayName = "Health"),
	AT_Mana UMETA(DisplayName = "Mana"),
	AT_Adrenaline UMETA(DisplayName = "Adrenaline"),

	AT_MAX UMETA(DisplayName = "DefaultMAX")
};


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class WOG_API UWOGAttributesComponent : public UWOGBaseActorComponent
{
	GENERATED_BODY()
public:
	UWOGAttributesComponent();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Attributes, meta = (AllowPrivateAccess = "true"), Replicated)
	float MaxHealth;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Attributes, meta = (AllowPrivateAccess = "true"), Replicated)
	float Health;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Attributes, meta = (AllowPrivateAccess = "true"), Replicated)
	float HealthPercent;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Attributes, meta = (AllowPrivateAccess = "true"), Replicated)
	float MaxMana;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Attributes, meta = (AllowPrivateAccess = "true"), Replicated)
	float Mana;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Attributes, meta = (AllowPrivateAccess = "true"), Replicated)
	float ManaPercent;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Attributes, meta = (AllowPrivateAccess = "true"), Replicated)
	float MaxAdrenaline;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Attributes, meta = (AllowPrivateAccess = "true"), Replicated)
	float Adrenaline;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Attributes, meta = (AllowPrivateAccess = "true"), Replicated)
	float AdrenalinePercent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Attributes, meta = (AllowPrivateAccess = "true"))
	float UpdateFrequency = 1.f;

private:

	UFUNCTION()
	void UpdateHealth(float Value, AController* InstigatedBy);
	UFUNCTION()
	void UpdateMana(float Value);
	UFUNCTION()
	void UpdateAdranaline(float Value);

	FTimerHandle PassiveUpdateTimer;
	FTimerDelegate PassiveUpdateTimerDelegate;

	UFUNCTION(Server, reliable)
	void Server_InitStats();

public:
	UFUNCTION(BlueprintCallable, Server, reliable)
	void Server_UpdateHealth(float Value, AController* InstigatedBy);

	UFUNCTION(BlueprintCallable, Server, reliable)
	void Server_UpdateMana(float Value);

	UFUNCTION(BlueprintCallable, Server, reliable)
	void Server_UpdateAdrenaline(float Value);

	UFUNCTION(BlueprintCallable, Server, reliable)
	void Server_PassiveAttributeUpdate(EAttributeType AttributeToUpdate, float Value);


	FORCEINLINE float GetHealthPercent() const { return HealthPercent; }
	FORCEINLINE float GetManaPercent() const { return ManaPercent; }
	FORCEINLINE float GetAdrenalinePercent() const { return AdrenalinePercent; }
};
