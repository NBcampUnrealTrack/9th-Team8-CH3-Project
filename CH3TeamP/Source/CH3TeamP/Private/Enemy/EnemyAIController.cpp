#include "Enemy/EnemyAIController.h"
#include "Enemy/BaseEnemy.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "Engine/Engine.h"

AEnemyAIController::AEnemyAIController()
{
	// Tick 함수를 사용하기 위해 활성화
	// 공격 범위 체크와 추적 로직을 매 프레임 확인해야 하므로 true로 설정
	PrimaryActorTick.bCanEverTick = true;
}

void AEnemyAIController::BeginPlay()
{
	Super::BeginPlay();
	
	// 0번 플레이어를 가져와 추적 대상으로 저장
	// 현재는 싱글 플레이 기준이라 0번 플레이어를 사용
	TargetPlayer = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	ControlledEnemy = Cast<ABaseEnemy>(GetPawn());
	
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			3.0f,
			FColor::Green,
			ControlledEnemy ? TEXT("ControlledEnemy OK") : TEXT("ControlledEnemy NULL")
			);
		
		GEngine->AddOnScreenDebugMessage(
			-1,
			3.0f,
			FColor::Yellow,
			TargetPlayer ? TEXT("TargetPlayer OK") : TEXT("Target NULL")
			);
	}
}

void AEnemyAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!ControlledEnemy)
	{
		ControlledEnemy = Cast<ABaseEnemy>(GetPawn());
	}

	if (!ControlledEnemy || !TargetPlayer)
	{
		if (ControlledEnemy && ControlledEnemy->GetEnemyState() != EEnemyState::Idle)
		{
			ControlledEnemy->SetEnemyState(EEnemyState::Idle);
		}

		StopMovement();
		return;
	}

	const float DistanceToPlayer =
		FVector::Dist(ControlledEnemy->GetActorLocation(), TargetPlayer->GetActorLocation());

	const float EnemyAttackRange = ControlledEnemy->GetAttackRange();
	const float EnemyAttackCooldown = ControlledEnemy->GetAttackCooldown();

	if (DistanceToPlayer > EnemyAttackRange)
	{
		if (ControlledEnemy->GetEnemyState() != EEnemyState::Chase)
		{
			ControlledEnemy->SetEnemyState(EEnemyState::Chase);
		}

		MoveToActor(TargetPlayer, EnemyAttackRange);
	}
	else
	{
		if (ControlledEnemy->GetEnemyState() != EEnemyState::Attack)
		{
			ControlledEnemy->SetEnemyState(EEnemyState::Attack);
		}

		StopMovement();

		const float CurrentTime = GetWorld()->GetTimeSeconds();

		if (CurrentTime - LastAttackTime >= EnemyAttackCooldown)
		{
			ControlledEnemy->AttackTarget(TargetPlayer);
			LastAttackTime = CurrentTime;
		}
	}
}