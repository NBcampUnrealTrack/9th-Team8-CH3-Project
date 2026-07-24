#include "Enemy/EnemyAIController.h"
#include "Enemy/BaseEnemy.h"

#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "Navigation/PathFollowingComponent.h"


AEnemyAIController::AEnemyAIController()
{
	// 플레이어 추적과 공격 거리 확인을 위해 Tick 활성화
	PrimaryActorTick.bCanEverTick = true;
}


void AEnemyAIController::BeginPlay()
{
	Super::BeginPlay();

	// 싱글 플레이 기준 0번 플레이어를 추적 대상으로 저장
	TargetPlayer = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
}


void AEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// 실제로 빙의한 Pawn을 ABaseEnemy로 변환
	ControlledEnemy = Cast<ABaseEnemy>(InPawn);

	// 새로운 적에 빙의했으므로 이동 및 공격 시간을 초기화
	LastMoveRequestTime = -1000.0f;
	LastAttackTime = -1000.0f;
}


void AEnemyAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	/*
	 * 1. 현재 조종 중인 Pawn 확인
	 */
	APawn* ControlledPawn = GetPawn();

	if (!IsValid(ControlledPawn))
	{
		ControlledEnemy = nullptr;
		StopMovement();
		return;
	}

	/*
	 * 2. 조종 중인 Pawn이 변경됐거나
	 * ControlledEnemy가 유효하지 않다면 다시 변환
	 */
	if (!IsValid(ControlledEnemy) ||
		ControlledEnemy != ControlledPawn)
	{
		ControlledEnemy = Cast<ABaseEnemy>(ControlledPawn);
	}

	if (!IsValid(ControlledEnemy))
	{
		StopMovement();
		return;
	}

	/*
	 * 3. 플레이어 확인
	 *
	 * BeginPlay 시점에 플레이어를 찾지 못했거나
	 * 기존 플레이어 Pawn이 제거됐다면 다시 찾습니다.
	 */
	if (!IsValid(TargetPlayer))
	{
		TargetPlayer =
			UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

		if (!IsValid(TargetPlayer))
		{
			if (ControlledEnemy->GetEnemyState() !=
				EEnemyState::Idle)
			{
				ControlledEnemy->SetEnemyState(
					EEnemyState::Idle
				);
			}

			StopMovement();
			return;
		}
	}

	/*
	 * 4. 적과 플레이어 사이의 거리 계산
	 */
	const float DistanceToPlayer = FVector::Dist(
		ControlledEnemy->GetActorLocation(),
		TargetPlayer->GetActorLocation()
	);

	const float EnemyAttackRange =
		ControlledEnemy->GetAttackRange();

	const float EnemyAttackCooldown =
		ControlledEnemy->GetAttackCooldown();

	/*
	 * 5. 공격 범위 밖이면 플레이어 추적
	 */
	if (DistanceToPlayer > EnemyAttackRange)
	{
		if (ControlledEnemy->GetEnemyState() !=
			EEnemyState::Chase)
		{
			ControlledEnemy->SetEnemyState(
				EEnemyState::Chase
			);
		}

		const float CurrentTime =
			GetWorld()->GetTimeSeconds();

		/*
		 * 이미 이동 중이라면 새로운 MoveTo 요청을 보내지 않습니다.
		 *
		 * 첫 이동 요청이 실패했을 경우에는 0.5초마다
		 * 다시 이동을 요청합니다.
		 */
		const bool bIsCurrentlyMoving =
			GetMoveStatus() == EPathFollowingStatus::Moving;

		const bool bCanRetryMove =
			CurrentTime - LastMoveRequestTime >= 0.5f;

		if (!bIsCurrentlyMoving && bCanRetryMove)
		{
			LastMoveRequestTime = CurrentTime;

			const float MoveAcceptanceRadius =
				FMath::Max(
					50.0f,
					EnemyAttackRange * 0.4f
				);

			MoveToActor(
				TargetPlayer,
				MoveAcceptanceRadius
			);
		}
	}
	/*
	 * 6. 공격 범위 안이면 이동을 멈추고 공격
	 */
	else
	{
		if (ControlledEnemy->GetEnemyState() !=
			EEnemyState::Attack)
		{
			ControlledEnemy->SetEnemyState(
				EEnemyState::Attack
			);
		}

		StopMovement();

		const float CurrentTime =
			GetWorld()->GetTimeSeconds();

		if (CurrentTime - LastAttackTime >=
			EnemyAttackCooldown)
		{
			ControlledEnemy->AttackTarget(TargetPlayer);
			LastAttackTime = CurrentTime;
		}
	}
}