#include "Components/StaminaComponent.h"

UCH3StaminaComponent::UCH3StaminaComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


void UCH3StaminaComponent::BeginPlay()
{
	Super::BeginPlay();

	CurrentStamina = MaxStamina;
}

void UCH3StaminaComponent::ConsumeStamina(float DeltaTime)
{
	CurrentStamina -= DrainRate * DeltaTime;
	CurrentStamina = FMath::Clamp(CurrentStamina, 0.f, MaxStamina);
}

void UCH3StaminaComponent::RecoverStamina(float DeltaTime)
{
	if (CurrentStamina >= MaxStamina)
	{
		return;
	}

	CurrentStamina += RecoverRate * DeltaTime;
	CurrentStamina = FMath::Clamp(CurrentStamina, 0.f, MaxStamina);
}

bool UCH3StaminaComponent::CanSprint() const
{
	return CurrentStamina > 0.f;
}

float UCH3StaminaComponent::GetStaminaPercent() const
{
	return CurrentStamina / MaxStamina;

}

float UCH3StaminaComponent::GetCurrentStamina() const
{
	return CurrentStamina;
}

float UCH3StaminaComponent::GetMaxStamina() const
{
	return MaxStamina;
}

