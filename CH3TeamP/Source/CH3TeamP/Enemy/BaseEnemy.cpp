#include "BaseEnemy.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/Engine.h"

ABaseEnemy::ABaseEnemy()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ABaseEnemy::BeginPlay()
{
	Super::BeginPlay();
	
	CurrentHealth = MaxHealth;
	GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
}

void ABaseEnemy::TakeEnemyDamage(float DamageAmount)
{
	if (bIsDead)
	{
		return;
	}
	
	const float FinalDamage = FMath::Max(DamageAmount - Defense, 0.f);
	CurrentHealth -= FinalDamage;
	
	if (CurrentHealth <= 0.f)
	{
		Die();
	}
}

void ABaseEnemy::Attack()
{
	if (bIsDead)
	{
		return;
	}
	
	SetEnemyState(EEnemyState::Attack);
	
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Red, TEXT("Enemy Attack"));
	}
}

void ABaseEnemy::Die()
{
	if (bIsDead)
	{
		return;
	}
	
	bIsDead = true;
	CurrentHealth = 0.f;
	SetEnemyState(EEnemyState::Dead);
}

void ABaseEnemy::SetEnemyState(EEnemyState NewState)
{
	EnemyState = NewState;
}



