#include "Enemy/BaseEnemy.h"
#include "Components/HealthComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/Engine.h"

ABaseEnemy::ABaseEnemy()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ABaseEnemy::BeginPlay()
{
	Super::BeginPlay();

	// 이동 속도를 CharacterMovement에 적용
	GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
}

void ABaseEnemy::TakeEnemyDamage(int32 DamageAmount)
{
	// 이미 죽은 상태면 더 이상 데미지를 받지 않음
	if (!HealthComponent || HealthComponent->bIsDead)
	{
		return;
	}
	
	// 방어력을 반영한 최종 데미지 계산
	const int32 FinalDamage = FMath::Max(0, DamageAmount - (int32)Defense);
	
	HealthComponent->ApplyDamage(FinalDamage);
	
	// 피격 확인용 디버그
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, TEXT("Enemy Hit"));
	}
	
	// 체력이 0 이하가 되면 사망 처리
	if (HealthComponent->bIsDead)
	{
		Die();
	}
}

void ABaseEnemy::Attack()
{
	if (!HealthComponent || HealthComponent->bIsDead)
	{
		return;
	}
	
	// 공격 상태로 전환
	SetEnemyState(EEnemyState::Attack);
	
	// 현재는 공격 호출 확인용 디버그 메시지
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Red, TEXT("Enemy Attack"));
	}
}

void ABaseEnemy::AttackTarget(AActor* Target)
{
	if (!HealthComponent || HealthComponent->bIsDead || !Target)
	{
		return;
	}
	
	// 공격 상태 전환
	Attack();
	
	// 타겟의 HealthComponent 찾아서 데미지 적용
	UHealthComponent* TargetHealth = Target->FindComponentByClass<UHealthComponent>();
	if (TargetHealth)
	{
		TargetHealth->ApplyDamage(AttackDamage);
	}
}

void ABaseEnemy::Die()
{
	if (!HealthComponent || !HealthComponent->bIsDead)
	{
		return;
	}
	
	SetEnemyState(EEnemyState::Dead);
	
	// 이동 중지
	GetCharacterMovement()->DisableMovement();
	
	// 사망 확인용 디버그
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, TEXT("Enemy Dead"));
	}
}

void ABaseEnemy::SetEnemyState(EEnemyState NewState)
{
	EnemyState = NewState;
}

EEnemyState ABaseEnemy::GetEnemyState() const
{
	return EnemyState;
}

float ABaseEnemy::GetAttackRange() const
{
	return AttackRange;
}

float ABaseEnemy::GetAttackCooldown() const
{
	return AttackCooldown;
}