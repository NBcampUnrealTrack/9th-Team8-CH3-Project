// Copyright Epic Games, Inc. All Rights Reserved.
// 게임 진행 표시/공유 데이터 (담당: 김유탁 / 게임모드)
// UI(김민석)는 이 GameState의 델리게이트에 바인딩하고, 값은 Getter로 읽습니다.
// 값 변경은 GameMode(서버 권한)에서만 수행합니다.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "CH3GameplayTypes.h"
#include "CH3GameState.generated.h"

// === UI 바인딩용 델리게이트 (블루프린트에서 바인딩 가능) ===
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGamePlayStateChanged, EGamePlayState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnScoreChanged, int32, NewScore);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWaveChanged, int32, CurrentWave, int32, TotalWaves);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemiesRemainingChanged, int32, Remaining);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWaveTimeChanged, float, TimeRemaining);

UCLASS()
class ACH3GameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	ACH3GameState();

	// ==================== UI 바인딩용 이벤트 ====================
	UPROPERTY(BlueprintAssignable, Category = "CH3|Events")
	FOnGamePlayStateChanged OnGamePlayStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "CH3|Events")
	FOnScoreChanged OnScoreChanged;

	UPROPERTY(BlueprintAssignable, Category = "CH3|Events")
	FOnWaveChanged OnWaveChanged;

	UPROPERTY(BlueprintAssignable, Category = "CH3|Events")
	FOnEnemiesRemainingChanged OnEnemiesRemainingChanged;

	UPROPERTY(BlueprintAssignable, Category = "CH3|Events")
	FOnWaveTimeChanged OnWaveTimeChanged;

	// ==================== 조회 (UI/그 외 파트에서 읽기) ====================
	UFUNCTION(BlueprintPure, Category = "CH3|State")
	EGamePlayState GetPlayState() const { return PlayState; }

	UFUNCTION(BlueprintPure, Category = "CH3|State")
	int32 GetTotalScore() const { return TotalScore; }

	UFUNCTION(BlueprintPure, Category = "CH3|State")
	int32 GetCurrentWave() const { return CurrentWave; }

	UFUNCTION(BlueprintPure, Category = "CH3|State")
	int32 GetTotalWaves() const { return TotalWaves; }

	UFUNCTION(BlueprintPure, Category = "CH3|State")
	int32 GetEnemiesRemaining() const { return EnemiesRemaining; }

	UFUNCTION(BlueprintPure, Category = "CH3|State")
	float GetWaveTimeRemaining() const { return WaveTimeRemaining; }

	// ==================== 변경 (GameMode 전용 진입점) ====================
	// GameMode가 호출 → 값 갱신 + 델리게이트 브로드캐스트를 한 번에 처리합니다.
	void SetPlayState(EGamePlayState NewState);
	void SetTotalScore(int32 NewScore);
	void AddScore(int32 Delta);
	void SetWave(int32 InCurrentWave, int32 InTotalWaves);
	void SetEnemiesRemaining(int32 Remaining);
	void SetWaveTimeRemaining(float TimeRemaining);

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(ReplicatedUsing = OnRep_PlayState)
	EGamePlayState PlayState = EGamePlayState::MainMenu;

	UPROPERTY(ReplicatedUsing = OnRep_TotalScore)
	int32 TotalScore = 0;

	UPROPERTY(ReplicatedUsing = OnRep_Wave)
	int32 CurrentWave = 0;

	UPROPERTY(Replicated)
	int32 TotalWaves = 0;

	UPROPERTY(ReplicatedUsing = OnRep_EnemiesRemaining)
	int32 EnemiesRemaining = 0;

	UPROPERTY(ReplicatedUsing = OnRep_WaveTime)
	float WaveTimeRemaining = 0.f;

	// 클라이언트 복제 콜백 (멀티플레이 대응). 스탠드얼론에서는 GameMode의 Setter가 직접 브로드캐스트합니다.
	UFUNCTION()
	void OnRep_PlayState();
	UFUNCTION()
	void OnRep_TotalScore();
	UFUNCTION()
	void OnRep_Wave();
	UFUNCTION()
	void OnRep_EnemiesRemaining();
	UFUNCTION()
	void OnRep_WaveTime();
};
