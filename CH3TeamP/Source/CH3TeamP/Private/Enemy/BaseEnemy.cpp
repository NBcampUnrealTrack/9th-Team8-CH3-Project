#include "Enemy/BaseEnemy.h"
#include "Components/HealthComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h" // 추가
#include "CH3TeamProjectGameMode.h" // 추가
#include "Characters/Player/PlayerCharacter.h"

ABaseEnemy::ABaseEnemy()
{
	PrimaryActorTick.bCanEverTick = false;

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
}

void ABaseEnemy::BeginPlay()
{
	Super::BeginPlay();

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
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

	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	if (!MovementComponent)
	{
		return;
	}

	MovementComponent->MaxWalkSpeed = MoveSpeed * HitSlowMultiplier;

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

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->MaxWalkSpeed = MoveSpeed;
	}
}

void ABaseEnemy::TakeEnemyDamage(int32 DamageAmount)
{
	if (!IsValid(HealthComponent) || HealthComponent->bIsDead)
	{
		return;
	}

	const int32 FinalDamage = FMath::Max(0, DamageAmount - static_cast<int32>(Defense));

	HealthComponent->ApplyDamage(FinalDamage);

	if (GEngine)
	{
		const FString DebugText = FString::Printf(
			TEXT("Enemy HP: %d / %d, Damage: %d"),
			HealthComponent->CurrentHP,
			HealthComponent->MaxHp,
			FinalDamage
		);
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, DebugText);
	}

	// HealthComponent::ApplyDamage는 HP만 깎고 bIsDead는 자동으로 true가 되지 않으므로 직접 체크
	if (HealthComponent->CurrentHP <= 0)
	{
		HealthComponent->MarkDead();
		Die();
		return;
	}

	ApplyHitSlow();
}

void ABaseEnemy::Attack()
{
	if (!IsValid(HealthComponent) || HealthComponent->bIsDead)
	{
		return;
	}

	SetEnemyState(EEnemyState::Attack);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Red, TEXT("Enemy Attack"));
	}
}

void ABaseEnemy::AttackTarget(AActor* Target)
{
	if (!IsValid(HealthComponent) || HealthComponent->bIsDead || !IsValid(Target))
	{
		return;
	}

	Attack();

	UHealthComponent* TargetHealth = Target->FindComponentByClass<UHealthComponent>();
	if (IsValid(TargetHealth))
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

	// 죽은 뒤 더 이상 이동하지 않도록 처리
	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->DisableMovement();
	}

	// 죽은 뒤 플레이어나 총알과 계속 충돌하지 않도록 캡슐 충돌 제거
	if (UCapsuleComponent* EnemyCapsule = GetCapsuleComponent())
	{
		EnemyCapsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// 추가: 게임모드에 처치 통지
	// 킬카운트, 킬피드, 미니맵 적 개체 표시 갱신에 사용됨
	if (ACH3TeamProjectGameMode* GM = Cast<ACH3TeamProjectGameMode>(UGameplayStatics::GetGameMode(this)))
	{
		GM->NotifyEnemyKilled(MyEnemyType, GetController());
	}

	// WaveManager에도 사망 통지
	OnEnemyDied.Broadcast(this);

	// 플레이어에게 경험치 즉시 지급
	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(
		UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)
	);

	if (PlayerCharacter)
	{
		PlayerCharacter->AddEXP(ExpReward);
	}

	// AI 이동/공격 중단
	DetachFromControllerPendingDestroy();

	// 시체가 일정 시간 뒤 사라지게 처리
	SetLifeSpan(2.0f);

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