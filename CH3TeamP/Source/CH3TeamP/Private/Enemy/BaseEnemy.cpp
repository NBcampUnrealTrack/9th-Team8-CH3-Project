#include "Enemy/BaseEnemy.h"
#include "Components/HealthComponent.h"
#include "Components/CapsuleComponent.h" // 추가: 캡슐 충돌 끄기 위해 필요
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"

ABaseEnemy::ABaseEnemy()
{
	PrimaryActorTick.bCanEverTick = false;

	// HealthComponent를 Enemy가 직접 보유하도록 생성
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
}

void ABaseEnemy::BeginPlay()
{
	Super::BeginPlay();

	// 블루프린트에서 설정한 이동 속도를 실제 CharacterMovement에 적용
	GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
}

void ABaseEnemy::ApplyHitSlow()
{
	if (!bUseHitSlow || EnemyState == EEnemyState::Dead)
	{
		return;
	}

	GetCharacterMovement()->MaxWalkSpeed = MoveSpeed * HitSlowMultiplier;

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

	GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
}

void ABaseEnemy::TakeEnemyDamage(int32 DamageAmount)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			1.0f,
			FColor::Yellow,
			TEXT("TakeEnemyDamage Called")
		);
	}

	// 이미 죽었거나 HealthComponent가 없으면 데미지 무시
	if (!HealthComponent || HealthComponent->bIsDead)
	{
		return;
	}

	// 방어력을 적용한 최종 데미지
	const int32 FinalDamage = FMath::Max(0, DamageAmount - static_cast<int32>(Defense));

	HealthComponent->ApplyDamage(FinalDamage);

	// 피격 시 슬로우 적용
	ApplyHitSlow();

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			1.5f,
			FColor::Cyan,
			FString::Printf(
				TEXT("Damage: %d / Final: %d / HP: %d"),
				DamageAmount,
				FinalDamage,
				HealthComponent->CurrentHP
			)
		);
	}

	// ApplyDamage는 HP만 깎고 bIsDead를 true로 만들지 않으므로 여기서 직접 처리
	if (HealthComponent->CurrentHP <= 0)
	{
		HealthComponent->MarkDead();
		Die();
	}
}

void ABaseEnemy::Attack()
{
	if (!HealthComponent || HealthComponent->bIsDead)
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
	if (!HealthComponent || HealthComponent->bIsDead || !Target)
	{
		return;
	}

	Attack();

	UHealthComponent* TargetHealth = Target->FindComponentByClass<UHealthComponent>();
	if (TargetHealth)
	{
		TargetHealth->ApplyDamage(AttackDamage);
	}
}

void ABaseEnemy::Die()
{
	// HealthComponent가 없거나 아직 죽은 상태가 아니면 사망 처리하지 않음
	if (!HealthComponent || !HealthComponent->bIsDead)
	{
		return;
	}

	// AnimBP에서 Death 상태로 넘어갈 수 있도록 EnemyState 변경
	SetEnemyState(EEnemyState::Dead);

	// 죽은 뒤 더 이상 이동하지 않도록 처리
	GetCharacterMovement()->DisableMovement();

	// 죽은 뒤 플레이어나 총알과 계속 충돌하지 않도록 캡슐 충돌 제거
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 혹시 AI가 계속 MoveTo/Attack을 시도하지 않도록 컨트롤러 분리
	DetachFromControllerPendingDestroy();

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, TEXT("Enemy Dead"));
	}

	// Death 애니메이션이 보일 시간을 준 뒤 액터 삭제
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