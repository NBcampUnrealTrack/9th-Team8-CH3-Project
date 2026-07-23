#include "Components/HealthComponent.h"
#include "Characters/Player/PlayerCharacter.h"

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

		AActor* Owner = GetOwner();
		if (Owner)
		{
			if (APlayerCharacter* Player = Cast<APlayerCharacter>(Owner))
			{
				Player->OnDeathAnimation();
			}
		}
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