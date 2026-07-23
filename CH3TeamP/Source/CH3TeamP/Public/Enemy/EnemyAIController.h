#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyAIController.generated.h"


class APawn;
class ABaseEnemy;


UCLASS()
class CH3TEAMP_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
	AEnemyAIController();

protected:
	// 게임 시작 시 호출
	virtual void BeginPlay() override;

	// AIController가 Pawn에 빙의할 때 호출
	virtual void OnPossess(APawn* InPawn) override;

	// 매 프레임 호출
	virtual void Tick(float DeltaTime) override;

protected:
	// 현재 추적할 플레이어
	UPROPERTY()
	APawn* TargetPlayer = nullptr;

	// 현재 AIController가 조종하는 적
	UPROPERTY()
	ABaseEnemy* ControlledEnemy = nullptr;

	// 마지막 공격 시각
	float LastAttackTime = -1000.0f;
};