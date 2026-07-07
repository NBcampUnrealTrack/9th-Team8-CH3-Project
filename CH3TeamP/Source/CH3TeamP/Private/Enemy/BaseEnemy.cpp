#include "Enemy/BaseEnemy.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/Engine.h"

ABaseEnemy::ABaseEnemy()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ABaseEnemy::BeginPlay()
{
	Super::BeginPlay();
	
	// 시작 시 현재 체력을 최대 체력으로 초기화
	CurrentHealth = MaxHealth;
	
	// 이동 속도를 CharacterMovement에 적용
	GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
}

void ABaseEnemy::TakeEnemyDamage(float DamageAmount)
{
	// 이미 죽은 상태면 더 이상 데미지를 받지 않음
	if (bIsDead)
	{
		return;
	}
	
	// 방어력을 반영한 최종 데미지 계산
	const float FinalDamage = FMath::Max(DamageAmount - Defense, 0.f);
	
	// 현재 체력 감소
	CurrentHealth -= FinalDamage;
	
	// 체력이 0 이하가 되면 사망 처리
	if (CurrentHealth <= 0.f)
	{
		Die();
	}
}

void ABaseEnemy::Attack()
{
	// 죽은 상태면 공격 불가
	if (bIsDead)
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

void ABaseEnemy::Die()
{
	// 이미 죽었으면 중복 사망 처리 방지
	if (bIsDead)
	{
		return;
	}
	
	bIsDead = true;
	CurrentHealth = 0.f;
	
	// 사망 상태로 변경
	SetEnemyState(EEnemyState::Dead);
	
	// 나중에 여기에 사망 애니메이션, 충돌 비활성화
	// 일정 시간 후 제거 같은 로직 추가할 수 있음
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