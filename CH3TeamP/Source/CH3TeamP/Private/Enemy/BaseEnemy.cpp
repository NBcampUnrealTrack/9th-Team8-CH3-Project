#include "Enemy/BaseEnemy.h"
#include "Components/HealthComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Items/ExpPickup.h"

ABaseEnemy::ABaseEnemy()
{
	PrimaryActorTick.bCanEverTick = false;
	
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
}

void ABaseEnemy::BeginPlay()
{
	Super::BeginPlay();

	// 이동 속도를 CharacterMovement에 적용
	GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
}

void ABaseEnemy::ApplyHitSlow()
{
	if (!bUseHitSlow || EnemyState == EEnemyState::Dead)
	{
		return;
	}
	
	if (!GetCharacterMovement())
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
	
	if (!GetCharacterMovement())
	{
		return;
	}
	
	GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
}

void ABaseEnemy::TakeEnemyDamage(int32 DamageAmount)
{
	// 디버깅 확인용
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			1.0f,
			FColor::Yellow,
			TEXT("TakeEnemyDamage Called")
			);
	}
	
	// 이미 죽은 상태면 더 이상 데미지를 받지 않음
	if (!HealthComponent || HealthComponent->bIsDead)
	{
		return;
	}
	
	// 방어력을 반영한 최종 데미지 계산
	const int32 FinalDamage = FMath::Max(0, DamageAmount - (int32)Defense);
	
	HealthComponent->ApplyDamage(FinalDamage);
	
	ApplyHitSlow();
	
	// 피격 확인용 디버그
	if (GEngine)
	{
		const FString DebugMessage = FString::Printf(
			TEXT("Enemy Hit / Damage: %d / HP: %d"),
			FinalDamage,
			HealthComponent->CurrentHP
			);
	}
	
	// 체력이 0 이하가 되면 사망 처리
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
	if (!HealthComponent || HealthComponent->bIsDead || !IsValid(Target))
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
	GetCharacterMovement()->DisableMovement();
	
	if (ExpPickupClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		
		const FVector SpawnLocation = GetActorLocation() + FVector(0.f, 0.f, 30.f);
		const FRotator SpawnRotation = FRotator::ZeroRotator;
		
		AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(
			ExpPickupClass,
			SpawnLocation,
			SpawnRotation,
			SpawnParams);
		
		AExpPickup* ExpPickup = Cast<AExpPickup>(SpawnedActor);
		if (ExpPickup)
		{
			ExpPickup->ExpAmount = ExpReward;
		}
	}
	
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