#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyAIController.generated.h"

// APawn 클래스를 미리 알려주는 전방 선언
// 헤더에서는 포인터만 쓸 거라 전체 include 대신 이렇게 가볍게 선언 가능
class APawn;

UCLASS()
class CH3TEAMP_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()
	
public:
	AEnemyAIController();
	
	// 매 프레임 호출되는 함수
	// 플레이어와의 거리 체크, 추적, 공격 가능 여부 판단을 여기서 수행
	virtual void Tick(float DeltaTime) override;
	
protected:
	// 게임 시작 시 한 번 호출되는 함수
	// 플레이어를 찾아서 추적 대상으로 저장하는 데 사용
	virtual void BeginPlay() override;
	
protected:
	// 현재 추적할 플레이어를 저장하는 포인터
	UPROPERTY()
	APawn* TargetPlayer;
	
	// 마지막으로 공격한 시각
	// 현재 시간과 비교해서 쿨타임이 지났는지 판단
	float LastAttackTime = -1000.0f;
};
