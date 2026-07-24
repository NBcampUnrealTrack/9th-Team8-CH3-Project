// Copyright Epic Games, Inc. All Rights Reserved.
// 게임 모드 (담당: 김유탁)
// 역할: 게임 상태 머신 / 웨이브(생존) 진행 / 플레이어 주변 연속 스폰 /
//       점수 / 승패 판정(승리 조건 선택식) / 레벨업 강화 흐름 제어.
//
// [협업 진입점 요약]  다른 파트는 아래 UFUNCTION 만 호출하면 됩니다.
//   - 적 AI(성태현)   : NotifyEnemyKilled()      // 몬스터 사망 통지(보스 처치 승리도 이걸로 감지)
//   - 전투(전병규)     : NotifyPlayerDied()        // 플레이어 사망
//                       NotifyPlayerLevelUp()      // 레벨업 발생
//   - UI(김민석)       : ConfirmUpgradeSelection() // 강화 카드 선택 결과
//                       (표시는 ACH3GameState 델리게이트 바인딩)
//
// [게임 상태를 즉석에서 바꾸는 치트 콘솔 명령] (~ 콘솔에 입력)
//   CH3.WinNow / CH3.LoseNow / CH3.SkipWave / CH3.SetWaveTime <초> / CH3.SpawnBoss
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CH3GameplayTypes.h"
#include "CH3TeamProjectGameMode.generated.h"

class ACH3GameState;
class IConsoleObject;

// 강화 카드 후보가 제시될 때 UI로 보내는 이벤트 (총 3장, 힐 섞임)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUpgradeCardsPresented, const TArray<EUpgradeType>&, Cards);
// 강화가 선택 확정되어 실제 효과를 적용해야 할 때 전투 로직으로 보내는 이벤트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUpgradeConfirmed, EUpgradeType, ChosenUpgrade, AController*, ForPlayer);
	// 추가 : UI담당 - 김민석 : [필수과제] 킬 피드를 위해 수정.(07/23)
	// 적 처치 확정 시 UI로 보내는 이벤트 (킬 피드 표시용)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemyKilledNotify, EEnemyType, KilledEnemyType);

UCLASS(minimalapi)
class ACH3TeamProjectGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ACH3TeamProjectGameMode();

	//========================================================================
	// 다른 파트에서 호출하는 진입점 (Interface Contract)
	//========================================================================

	/** [적 AI → 게임모드] 몬스터 사망 통지. 점수 가산 + 생존 적 수 감소. 보스면 승리 조건 판정. */
	UFUNCTION(BlueprintCallable, Category = "CH3|GameMode")
	void NotifyEnemyKilled(EEnemyType EnemyType, AController* Killer);

	/** [전투 → 게임모드] 플레이어 캐릭터 사망 시 호출. → 게임 오버. */
	UFUNCTION(BlueprintCallable, Category = "CH3|GameMode")
	void NotifyPlayerDied(AController* PlayerController);

	/** [전투 → 게임모드] 레벨업 발생 시 호출. → 게임 일시정지 + 강화 카드 제시. */
	UFUNCTION(BlueprintCallable, Category = "CH3|GameMode")
	void NotifyPlayerLevelUp(AController* PlayerController, int32 NewLevel);

	/** [UI → 게임모드] 강화 카드 선택 확정 시 호출. → 효과 적용 위임 + 게임 재개. */
	UFUNCTION(BlueprintCallable, Category = "CH3|GameMode")
	void ConfirmUpgradeSelection(EUpgradeType ChosenUpgrade);

	//========================================================================
	// 게임 흐름 제어 (메뉴/UI에서 호출)
	//========================================================================

	UFUNCTION(BlueprintCallable, Category = "CH3|GameMode")
	void StartGame();

	UFUNCTION(BlueprintCallable, Category = "CH3|GameMode")
	void RestartGame();

	UFUNCTION(BlueprintPure, Category = "CH3|GameMode")
	EGamePlayState GetGamePlayState() const;

	//========================================================================
	// 디버그/치트 (콘솔 명령 + 블루프린트에서도 호출 가능)
	//========================================================================

	/** 즉시 게임 클리어. (콘솔: CH3.WinNow) */
	UFUNCTION(BlueprintCallable, Category = "CH3|Debug")
	void WinNow();

	/** 즉시 게임 오버. (콘솔: CH3.LoseNow) */
	UFUNCTION(BlueprintCallable, Category = "CH3|Debug")
	void LoseNow();

	/** 현재 웨이브를 즉시 넘긴다(다음 웨이브 or 마지막이면 승리 판정). (콘솔: CH3.SkipWave) */
	UFUNCTION(BlueprintCallable, Category = "CH3|Debug")
	void SkipWave();

	/** 현재 웨이브 남은 시간을 강제로 설정(초). (콘솔: CH3.SetWaveTime <초>) */
	UFUNCTION(BlueprintCallable, Category = "CH3|Debug")
	void DebugSetWaveTime(float Seconds);

	/** 보스를 즉시 소환하고 보스전 페이즈로 전환. (콘솔: CH3.SpawnBoss) */
	UFUNCTION(BlueprintCallable, Category = "CH3|Debug")
	void SpawnBossNow();

	//========================================================================
	// 협업 이벤트 (UI / 전투가 바인딩)
	//========================================================================

	UPROPERTY(BlueprintAssignable, Category = "CH3|Events")
	FOnUpgradeCardsPresented OnUpgradeCardsPresented;

	UPROPERTY(BlueprintAssignable, Category = "CH3|Events")
	FOnUpgradeConfirmed OnUpgradeConfirmed;
	
		// 추가 : UI담당 - 김민석 : [필수과제] 킬 카운트를 위해 수정.(07/23)
	UPROPERTY(BlueprintAssignable, Category = "CH3|Events")
	FOnEnemyKilledNotify OnEnemyKilledNotify;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	//========================================================================
	// 설정값 (에디터/설정에서 조정 — "간단하게 바꾸는" 노브들)
	//========================================================================

	/** 승리(클리어) 조건. 타이머 생존 / 보스 처치 / 생존 후 보스 중 선택. */
	UPROPERTY(EditDefaultsOnly, Category = "CH3|Rules")
	EGameWinCondition WinCondition = EGameWinCondition::SurviveAllWaves;

	/** BeginPlay 시 자동으로 게임을 시작할지 (메인메뉴 UI 완성 전 테스트용). */
	UPROPERTY(EditDefaultsOnly, Category = "CH3|Rules")
	bool bAutoStartOnBeginPlay = true;

	/** 총 웨이브 수. */
	UPROPERTY(EditDefaultsOnly, Category = "CH3|Rules", meta = (ClampMin = "1"))
	int32 TotalWaves = 5;

	/** 웨이브 1개의 지속 시간(초). 기본 120초(2분). */
	UPROPERTY(EditDefaultsOnly, Category = "CH3|Rules", meta = (ClampMin = "1"))
	float WaveDuration = 120.f;

	/** 웨이브 사이 대기 시간(초). 0 = 끊김 없이 바로 다음 웨이브. */
	UPROPERTY(EditDefaultsOnly, Category = "CH3|Rules", meta = (ClampMin = "0"))
	float IntermissionSeconds = 0.f;

	//---- 스폰 튜닝 ----
	/** 스폰 배치 간격(초). 기본 1초. */
	UPROPERTY(EditDefaultsOnly, Category = "CH3|Spawn", meta = (ClampMin = "0.1"))
	float SpawnInterval = 1.f;

	/** 한 번에 소환할 몬스터 수. 기본 3마리. */
	UPROPERTY(EditDefaultsOnly, Category = "CH3|Spawn", meta = (ClampMin = "1"))
	int32 SpawnPerBatch = 3;

	/** 동시에 존재할 수 있는 최대 몬스터 수. */
	UPROPERTY(EditDefaultsOnly, Category = "CH3|Spawn", meta = (ClampMin = "1"))
	int32 MaxConcurrentEnemies = 30;

	/** 웨이브가 오를수록 스폰을 더 빠르고 많게 자동 상향. 끄면 위 값 그대로 유지. */
	UPROPERTY(EditDefaultsOnly, Category = "CH3|Spawn")
	bool bScaleDifficultyByWave = false;

	/** 플레이어로부터 스폰 최소 거리(cm). 너무 가깝지 않게. */
	UPROPERTY(EditDefaultsOnly, Category = "CH3|Spawn", meta = (ClampMin = "0"))
	float MinSpawnDistance = 800.f;

	/** 플레이어로부터 스폰 최대 거리(cm). */
	UPROPERTY(EditDefaultsOnly, Category = "CH3|Spawn", meta = (ClampMin = "1"))
	float MaxSpawnDistance = 2000.f;

	//---- 데이터/클래스 ----
	/** 웨이브 구성 데이터 테이블 (Row: FWaveInfo). 없으면 위 노브로 자동 생성. */
	UPROPERTY(EditDefaultsOnly, Category = "CH3|Data")
	TObjectPtr<UDataTable> WaveTable = nullptr;

	/** 몬스터 종류별 스폰 클래스. AI(성태현)의 몬스터 BP를 여기에 연결. 비어있으면 스폰 생략(로그). */
	UPROPERTY(EditDefaultsOnly, Category = "CH3|Data")
	TMap<EEnemyType, TSubclassOf<AActor>> EnemyClasses;

	/** 몬스터 종류별 처치 점수. */
	UPROPERTY(EditDefaultsOnly, Category = "CH3|Data")
	TMap<EEnemyType, int32> ScorePerEnemyType;

	/** 제시할 카드 총 개수(힐 포함, 중복 없이 랜덤). 기본 3장. */
	UPROPERTY(EditDefaultsOnly, Category = "CH3|Rules", meta = (ClampMin = "1"))
	int32 UpgradeCardCount = 3;

private:
	//========================================================================
	// 내부 로직
	//========================================================================
	void TransitionTo(EGamePlayState NewState);
	void StartWave(int32 WaveNumber);
	void OnWaveTimeUp();
	void AdvanceToNextWave();
	void EnterBossPhase();
	void HandleGameOver();
	void HandleGameClear();

	bool IsFinalWave(int32 WaveNumber) const { return WaveNumber >= TotalWaves; }

	FWaveInfo GetWaveInfo(int32 WaveNumber) const;

	void SpawnBatch();
	bool SpawnSingleEnemy(EEnemyType Type);
	EEnemyType PickWeightedEnemyType(const FWaveInfo& Info) const;
	bool GetSpawnLocationAroundPlayer(FVector& OutLocation) const;

	void TickWaveTimer();
	TArray<EUpgradeType> RollUpgradeCards() const;
		// UI담당 - 김민석 : 탄환 변경 건을 위해 수정.(07/23)
		// 한 줄만 추가.
		// 탄환 계열 강화인지 판정. 나중에 탄환 종류가 늘어나면 이 함수 한 곳만 고치면 됨.
	static bool IsAmmoUpgrade(EUpgradeType Type);	
	int32 GetScoreForEnemy(EEnemyType EnemyType) const;

	ACH3GameState* GetCH3GameState() const;
	APawn* GetPlayerPawnSafe() const;

	void RegisterCheatCommands();
	void UnregisterCheatCommands();

	// --- 런타임 상태 ---
	EGamePlayState CurrentState = EGamePlayState::MainMenu;
	int32 ActiveWave = 0;
	int32 EnemiesAlive = 0;
	FWaveInfo ActiveWaveInfo;

	/** 보스전 진행 중(보스 처치로만 클리어). */
	bool bBossPhaseActive = false;

	EGamePlayState StateBeforePause = EGamePlayState::WaveInProgress;
	TWeakObjectPtr<AController> PendingUpgradeController;
	TArray<EUpgradeType> CurrentUpgradeCards;
	
	
		// UI 담당 - 김민석 추가 부분(07/23)
		// 마지막으로 선택된 탄환 강화. 같은 탄환이 다음 카드에 다시 나오지 않게 하는 용도.
		// MAX = 아직 탄환을 고른 적 없음. (Hidden 값이라 후보 풀에 절대 안 들어가서 센티널로 안전)
	EUpgradeType LastChosenAmmo = EUpgradeType::MAX;

	FTimerHandle IntermissionTimerHandle;
	FTimerHandle WaveTimerHandle;
	FTimerHandle SpawnTimerHandle;
	float RemainingWaveTime = 0.f;

	/** 등록한 콘솔 치트 명령들 (EndPlay에서 해제). */
	TArray<IConsoleObject*> ConsoleCommands;
};
