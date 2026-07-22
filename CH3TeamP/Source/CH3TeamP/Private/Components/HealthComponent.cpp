#include "Components/HealthComponent.h"

UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;   // 틱 필요 없음
}

void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	ResetHealth();   // 시작 시 풀피
}

void UHealthComponent::ResetHealth()
{
	CurrentHP = MaxHp;
	bIsDead = false;
}

void UHealthComponent::ApplyDamage(int32 Amount)
{
	if (bIsDead)
	{
		return;
	}

	CurrentHP = FMath::Max(0, CurrentHP - Amount);

	if (CurrentHP <= 0)
	{
		bIsDead = true;
	}
}

void UHealthComponent::MarkDead()
{
	bIsDead = true;
}

float UHealthComponent::GetHealthPercent() const
{
	return MaxHp > 0 ? (float)CurrentHP / MaxHp : 0.0f;   // HP바
}