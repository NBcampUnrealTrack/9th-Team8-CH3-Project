#include "Enemy/BaseEnemy.h"

#include "Components/HealthComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "CH3TeamProjectGameMode.h"
#include "Characters/Player/PlayerCharacter.h"


ABaseEnemy::ABaseEnemy()
{
	PrimaryActorTick.bCanEverTick = false;

	HealthComponent =
		CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
}


void ABaseEnemy::BeginPlay()
{
	Super::BeginPlay();

	if (UCharacterMovementComponent* MovementComponent =
		GetCharacterMovement())
	{
		MovementComponent->MaxWalkSpeed = MoveSpeed;
	}
}


void ABaseEnemy::ApplyHitSlow()
{
	if (!bUseHitSlow || EnemyState == EEnemyState::Dead)
	{
		return;
	}

	UCharacterMovementComponent* MovementComponent =
		GetCharacterMovement();

	if (!MovementComponent)
	{
		return;
	}

	MovementComponent->MaxWalkSpeed =
		MoveSpeed * HitSlowMultiplier;

	GetWorldTimerManager().ClearTimer(HitSlowTimerHandle);

	GetWorldTimerManager().SetTimer(
		HitSlowTimerHandle,
		this,
		&ABaseEnemy::ResetMoveSpeed,
		HitSlowDuration,
		false
	);
}


void ABaseEnemy::ResetMoveSpeed()
{
	if (EnemyState == EEnemyState::Dead)
	{
		return;
	}

	if (UCharacterMovementComponent* MovementComponent =
		GetCharacterMovement())
	{
		MovementComponent->MaxWalkSpeed = MoveSpeed;
	}
}


void ABaseEnemy::TakeEnemyDamage(int32 DamageAmount)
{
	// 이미 죽었거나 체력 컴포넌트가 유효하지 않으면 무시
	if (!IsValid(HealthComponent) || HealthComponent->bIsDead)
	{
		return;
	}

	// 방어력을 적용한 최종 피해량
	const int32 FinalDamage = FMath::Max(
		0,
		DamageAmount - static_cast<int32>(Defense)
	);

	HealthComponent->ApplyDamage(FinalDamage);

	// 체력이 0 이하라면 사망 처리
	if (HealthComponent->CurrentHP <= 0)
	{
		HealthComponent->MarkDead();
		Die();
		return;
	}

	// 살아 있는 경우에만 피격 슬로우 적용
	ApplyHitSlow();
}


void ABaseEnemy::Attack()
{
	if (!IsValid(HealthComponent) || HealthComponent->bIsDead)
	{
		return;
	}

	SetEnemyState(EEnemyState::Attack);
}


void ABaseEnemy::AttackTarget(AActor* Target)
{
	if (!IsValid(HealthComponent) ||
		HealthComponent->bIsDead ||
		!IsValid(Target))
	{
		return;
	}

	Attack();

	UHealthComponent* TargetHealth =
		Target->FindComponentByClass<UHealthComponent>();

	if (IsValid(TargetHealth))
	{
		TargetHealth->ApplyDamage(AttackDamage);
	}
}


void ABaseEnemy::Die()
{
	if (!IsValid(HealthComponent) || !HealthComponent->bIsDead)
	{
		return;
	}

	// 경험치와 킬 카운트가 중복 지급되는 것을 방지
	if (EnemyState == EEnemyState::Dead)
	{
		return;
	}

	SetEnemyState(EEnemyState::Dead);

	// 남아 있는 피격 슬로우 타이머 제거
	GetWorldTimerManager().ClearTimer(HitSlowTimerHandle);

	// 죽은 뒤 이동 중지
	if (UCharacterMovementComponent* MovementComponent =
		GetCharacterMovement())
	{
		MovementComponent->DisableMovement();
	}

	// 죽은 뒤 플레이어 및 총알과의 캡슐 충돌 제거
	if (UCapsuleComponent* EnemyCapsule =
		GetCapsuleComponent())
	{
		EnemyCapsule->SetCollisionEnabled(
			ECollisionEnabled::NoCollision
		);
	}

	/*
	 * 게임모드에 처치 통지
	 * 킬 카운트, 킬피드, 미니맵 갱신에 사용
	 *
	 * Controller를 분리하기 전에 전달해야 합니다.
	 */
	if (ACH3TeamProjectGameMode* GameMode =
		Cast<ACH3TeamProjectGameMode>(
			UGameplayStatics::GetGameMode(this)
		))
	{
		GameMode->NotifyEnemyKilled(
			MyEnemyType,
			GetController()
		);
	}

	// WaveManager에 적 사망 통지
	OnEnemyDied.Broadcast(this);

	// 플레이어에게 경험치 지급
	APlayerCharacter* PlayerCharacter =
		Cast<APlayerCharacter>(
			UGameplayStatics::GetPlayerCharacter(
				GetWorld(),
				0
			)
		);

	if (IsValid(PlayerCharacter))
	{
		PlayerCharacter->AddEXP(ExpReward);
	}

	// AI 이동 및 공격 중단
	DetachFromControllerPendingDestroy();

	// 사망 애니메이션 재생 시간을 준 뒤 액터 제거
	SetLifeSpan(2.0f);
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