#include "Enemy/EnemyAIController.h"
#include "Enemy/BaseEnemy.h"

#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "Engine/Engine.h"
#include "Navigation/PathFollowingComponent.h"


AEnemyAIController::AEnemyAIController()
{
    // 추적 및 공격 범위 확인을 위해 Tick 활성화
    PrimaryActorTick.bCanEverTick = true;
}


void AEnemyAIController::BeginPlay()
{
    Super::BeginPlay();

    // 싱글 플레이 기준 0번 플레이어를 추적 대상으로 저장
    TargetPlayer = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(
            -1,
            3.0f,
            TargetPlayer ? FColor::Green : FColor::Red,
            TargetPlayer
                ? TEXT("TargetPlayer OK")
                : TEXT("TargetPlayer NULL")
        );
    }
}


void AEnemyAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    // 실제 빙의된 Pawn을 ABaseEnemy로 변환
    ControlledEnemy = Cast<ABaseEnemy>(InPawn);

    if (GEngine)
    {
        if (ControlledEnemy)
        {
            GEngine->AddOnScreenDebugMessage(
                -1,
                5.0f,
                FColor::Green,
                TEXT("OnPossess: ControlledEnemy OK")
            );
        }
        else
        {
            const FString PawnClassName = InPawn
                ? InPawn->GetClass()->GetName()
                : TEXT("NULL");

            GEngine->AddOnScreenDebugMessage(
                -1,
                5.0f,
                FColor::Red,
                FString::Printf(
                    TEXT("OnPossess: Pawn is not BaseEnemy: %s"),
                    *PawnClassName
                )
            );
        }
    }
}


void AEnemyAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    /*
     * 1. 현재 조종 중인 Pawn 확인
     */
    APawn* ControlledPawn = GetPawn();

    if (!ControlledPawn)
    {
        ControlledEnemy = nullptr;

        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(
                1001,
                0.1f,
                FColor::Red,
                TEXT("GetPawn NULL")
            );
        }

        StopMovement();
        return;
    }

    /*
     * 2. 조종 중인 Pawn이 변경되었거나
     * ControlledEnemy가 설정되지 않았다면 다시 변환
     */
    if (!ControlledEnemy || ControlledEnemy != ControlledPawn)
    {
        ControlledEnemy = Cast<ABaseEnemy>(ControlledPawn);
    }

    if (!ControlledEnemy)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(
                1002,
                0.1f,
                FColor::Orange,
                FString::Printf(
                    TEXT("Pawn is not BaseEnemy: %s"),
                    *ControlledPawn->GetClass()->GetName()
                )
            );
        }

        StopMovement();
        return;
    }

    /*
     * 3. 플레이어 확인
     */
    if (!TargetPlayer)
    {
        TargetPlayer = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

        if (!TargetPlayer)
        {
            if (ControlledEnemy->GetEnemyState() != EEnemyState::Idle)
            {
                ControlledEnemy->SetEnemyState(EEnemyState::Idle);
            }

            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(
                    1003,
                    0.1f,
                    FColor::Yellow,
                    TEXT("TargetPlayer NULL")
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
     * 현재 거리와 공격 범위를 화면에 표시
     */
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(
            2000,
            0.1f,
            FColor::Cyan,
            FString::Printf(
                TEXT("Distance: %.0f | AttackRange: %.0f"),
                DistanceToPlayer,
                EnemyAttackRange
            )
        );
    }

    /*
     * 5. 공격 범위 밖이라면 플레이어 추적
     */
    if (DistanceToPlayer > EnemyAttackRange)
    {
        /*
         * Chase 상태에 처음 들어갈 때만 MoveToActor 호출
         *
         * MoveToActor는 움직이는 TargetPlayer의 위치를
         * 계속 갱신하므로 매 프레임 다시 호출할 필요가 없습니다.
         */
        if (ControlledEnemy->GetEnemyState() != EEnemyState::Chase)
        {
            ControlledEnemy->SetEnemyState(EEnemyState::Chase);

            const float MoveAcceptanceRadius =
                FMath::Max(50.0f, EnemyAttackRange * 0.4f);

            const EPathFollowingRequestResult::Type MoveResult =
                MoveToActor(
                    TargetPlayer,
                    MoveAcceptanceRadius
                );

            FString MoveResultText;
            FColor MoveResultColor = FColor::White;

            switch (MoveResult)
            {
            case EPathFollowingRequestResult::RequestSuccessful:
                MoveResultText =
                    TEXT("MoveTo: Request Successful");

                MoveResultColor = FColor::Green;
                break;

            case EPathFollowingRequestResult::AlreadyAtGoal:
                MoveResultText =
                    TEXT("MoveTo: Already At Goal");

                MoveResultColor = FColor::Yellow;
                break;

            case EPathFollowingRequestResult::Failed:
            default:
                MoveResultText =
                    TEXT("MoveTo: FAILED");

                MoveResultColor = FColor::Red;
                break;
            }

            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(
                    -1,
                    5.0f,
                    MoveResultColor,
                    MoveResultText
                );
            }
        }
    }
    /*
     * 6. 공격 범위 안이라면 이동을 멈추고 공격
     */
    else
    {
        if (ControlledEnemy->GetEnemyState() != EEnemyState::Attack)
        {
            ControlledEnemy->SetEnemyState(EEnemyState::Attack);
        }

        StopMovement();

        const float CurrentTime =
            GetWorld()->GetTimeSeconds();

        if (CurrentTime - LastAttackTime >= EnemyAttackCooldown)
        {
            ControlledEnemy->AttackTarget(TargetPlayer);
            LastAttackTime = CurrentTime;
        }
    }
}