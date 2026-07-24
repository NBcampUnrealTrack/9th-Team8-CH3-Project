#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StaminaComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CH3TEAMP_API UCH3StaminaComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCH3StaminaComponent();
	
protected:
	virtual void BeginPlay() override;

public:
	/** 스태미나 소모 */
	void ConsumeStamina(float DeltaTime);

	/** 스태미나 회복 */
	void RecoverStamina(float DeltaTime);

	/** 달릴 수 있는지 */
	bool CanSprint() const;

	/** 현재 스태미나 비율 */
	float GetStaminaPercent() const;

	/** 현재 스태미나 */
	float GetCurrentStamina() const;

	/** 최대 스태미나 */
	float GetMaxStamina() const;
	
	/** 최대 스태미나 증가 및 회복 */
	void IncreaseMaxStamina(float Amount)
	{
		MaxStamina += Amount;
		CurrentStamina = FMath::Clamp(CurrentStamina + Amount, 0.f, MaxStamina);
	}

private:

	/** 최대 스태미나 */
	UPROPERTY(EditAnywhere, Category="Stamina")
	float MaxStamina = 100.f;

	/** 현재 스태미나 */
	UPROPERTY(VisibleAnywhere, Category="Stamina")
	float CurrentStamina = 100.f;

	/** 초당 감소량 */
	UPROPERTY(EditAnywhere, Category="Stamina")
	float DrainRate = 20.f;

	/** 초당 회복량 */
	UPROPERTY(EditAnywhere, Category="Stamina")
	float RecoverRate = 15.f;
	

};
