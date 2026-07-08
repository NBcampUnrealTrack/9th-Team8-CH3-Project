#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CH3TEAMP_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UHealthComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Health")
	int32 MaxHp = 100;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Health")
	int32 CurrentHP = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Health")
	bool bIsDead = false;

	UFUNCTION(BlueprintCallable, Category="Health")
	void ResetHealth();

	UFUNCTION(BlueprintCallable, Category="Health")
	void ApplyDamage(int32 Amount);

	UFUNCTION(BlueprintCallable, Category="Health")
	void MarkDead();

	UFUNCTION(BlueprintPure, Category="Health")
	float GetHealthPercent() const;
};