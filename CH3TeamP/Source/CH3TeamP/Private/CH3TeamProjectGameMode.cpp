// Copyright Epic Games, Inc. All Rights Reserved.

#include "CH3TeamProjectGameMode.h"
// [드롭인 수정] 템플릿 캐릭터 include 제거 — 실제로 사용하지 않음.
// #include "CH3TeamProjectCharacter.h"
#include "CH3GameState.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "UObject/ConstructorHelpers.h"

DEFINE_LOG_CATEGORY_STATIC(LogCH3GameMode, Log, All);

ACH3TeamProjectGameMode::ACH3TeamProjectGameMode()
	: Super()
{
	// [드롭인 수정] First Person 템플릿 BP(BP_FirstPersonCharacter)를 참조하던 부분 비활성화.
	// 블랭크 프로젝트엔 이 애셋이 없으므로 GameModeBase 기본 폰(ADefaultPawn)이 스폰됩니다.
	// 팀에서 새 플레이어 폰(BP)을 만들면 아래 두 줄을 그 경로로 바꿔 활성화하세요.
	// static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/Path/To/BP_YourCharacter"));
	// DefaultPawnClass = PlayerPawnClassFinder.Class;

	GameStateClass = ACH3GameState::StaticClass();

	// 기본 점수 테이블
	ScorePerEnemyType.Add(EEnemyType::Normal, 10);
	ScorePerEnemyType.Add(EEnemyType::Rush, 20);
	ScorePerEnemyType.Add(EEnemyType::Tanker, 30);
	ScorePerEnemyType.Add(EEnemyType::Boss, 500);
}

void ACH3TeamProjectGameMode::BeginPlay()
{
	Super::BeginPlay();

	RegisterCheatCommands();

	TransitionTo(EGamePlayState::MainMenu);

	if (bAutoStartOnBeginPlay)
	{
		StartGame();
	}
}

void ACH3TeamProjectGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnregisterCheatCommands();
	Super::EndPlay(EndPlayReason);
}

EGamePlayState ACH3TeamProjectGameMode::GetGamePlayState() const
{
	return CurrentState;
}

ACH3GameState* ACH3TeamProjectGameMode::GetCH3GameState() const
{
	return GetWorld() ? GetWorld()->GetGameState<ACH3GameState>() : nullptr;
}

APawn* ACH3TeamProjectGameMode::GetPlayerPawnSafe() const
{
	return UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
}

//============================================================================
// 게임 흐름
//============================================================================

void ACH3TeamProjectGameMode::StartGame()
{
	TransitionTo(EGamePlayState::Starting);

	ActiveWave = 0;
	EnemiesAlive = 0;
	bBossPhaseActive = false;

	if (ACH3GameState* GS = GetCH3GameState())
	{
		GS->SetTotalScore(0);
		GS->SetWave(0, TotalWaves);
		GS->SetEnemiesRemaining(0);
	}

	UE_LOG(LogCH3GameMode, Log, TEXT("게임 시작. 총 웨이브=%d, 승리조건=%d"), TotalWaves, (int32)WinCondition);
	StartWave(1);
}

void ACH3TeamProjectGameMode::RestartGame()
{
	GetWorldTimerManager().ClearTimer(IntermissionTimerHandle);
	GetWorldTimerManager().ClearTimer(WaveTimerHandle);
	GetWorldTimerManager().ClearTimer(SpawnTimerHandle);

	UGameplayStatics::SetGamePaused(this, false);

	const FString LevelName = UGameplayStatics::GetCurrentLevelName(this);
	UGameplayStatics::OpenLevel(this, FName(*LevelName));
}

//============================================================================
// 웨이브 진행 (생존 + 연속 스폰)
//============================================================================

void ACH3TeamProjectGameMode::StartWave(int32 WaveNumber)
{
	ActiveWave = WaveNumber;
	ActiveWaveInfo = GetWaveInfo(WaveNumber);
	bBossPhaseActive = false;

	RemainingWaveTime = ActiveWaveInfo.WaveDurationSeconds;

	if (ACH3GameState* GS = GetCH3GameState())
	{
		GS->SetWave(WaveNumber, TotalWaves);
		GS->SetWaveTimeRemaining(RemainingWaveTime);
	}

	TransitionTo(EGamePlayState::WaveInProgress);

	UE_LOG(LogCH3GameMode, Log, TEXT("웨이브 %d 시작. %.0f초, %.1f초마다 %d마리, 동시최대 %d."),
		WaveNumber, ActiveWaveInfo.WaveDurationSeconds, ActiveWaveInfo.SpawnInterval,
		ActiveWaveInfo.SpawnPerBatch, ActiveWaveInfo.MaxConcurrentEnemies);

	// 승리 조건이 '보스 처치'면 마지막 웨이브 시작과 동시에 보스전 진입.
	if (WinCondition == EGameWinCondition::DefeatBoss && IsFinalWave(WaveNumber))
	{
		EnterBossPhase();
	}

	// 첫 배치 즉시 + 주기 스폰.
	SpawnBatch();
	GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &ACH3TeamProjectGameMode::SpawnBatch,
		ActiveWaveInfo.SpawnInterval, true);

	// 생존 카운트다운.
	GetWorldTimerManager().SetTimer(WaveTimerHandle, this, &ACH3TeamProjectGameMode::TickWaveTimer, 0.1f, true);
}

void ACH3TeamProjectGameMode::TickWaveTimer()
{
	RemainingWaveTime -= 0.1f;

	if (ACH3GameState* GS = GetCH3GameState())
	{
		GS->SetWaveTimeRemaining(RemainingWaveTime);
	}

	if (RemainingWaveTime <= 0.f)
	{
		GetWorldTimerManager().ClearTimer(WaveTimerHandle);
		OnWaveTimeUp();
	}
}

void ACH3TeamProjectGameMode::OnWaveTimeUp()
{
	// 보스전 중이면 시간이 다 돼도 클리어하지 않음(보스를 잡아야 함). 잡몹 스폰만 종료.
	if (bBossPhaseActive)
	{
		GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
		UE_LOG(LogCH3GameMode, Log, TEXT("보스전: 시간 종료, 잡몹 스폰 중단. 보스 처치 시 클리어."));
		return;
	}

	// 마지막 웨이브가 아니면 다음 웨이브로.
	if (!IsFinalWave(ActiveWave))
	{
		UE_LOG(LogCH3GameMode, Log, TEXT("웨이브 %d 생존 완료 → 다음 웨이브."), ActiveWave);
		AdvanceToNextWave();
		return;
	}

	// 마지막 웨이브 생존 완료 → 승리 조건에 따라 분기.
	switch (WinCondition)
	{
	case EGameWinCondition::SurviveAllWaves:
		UE_LOG(LogCH3GameMode, Log, TEXT("마지막 웨이브 생존 완료 → 게임 클리어."));
		HandleGameClear();
		break;

	case EGameWinCondition::SurviveThenDefeatBoss:
		UE_LOG(LogCH3GameMode, Log, TEXT("마지막 웨이브 생존 완료 → 보스 등장!"));
		EnterBossPhase();
		GetWorldTimerManager().ClearTimer(SpawnTimerHandle); // 보스전은 잡몹 스폰 중단(보스에 집중).
		break;

	case EGameWinCondition::DefeatBoss:
		// 이 케이스는 보스가 웨이브 시작부터 있어 bBossPhaseActive=true → 위에서 return 됨.
		break;
	}
}

void ACH3TeamProjectGameMode::AdvanceToNextWave()
{
	GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
	GetWorldTimerManager().ClearTimer(WaveTimerHandle);

	const int32 NextWave = ActiveWave + 1;
	if (IntermissionSeconds > 0.f)
	{
		TransitionTo(EGamePlayState::WaveIntermission);
		FTimerDelegate Del = FTimerDelegate::CreateUObject(this, &ACH3TeamProjectGameMode::StartWave, NextWave);
		GetWorldTimerManager().SetTimer(IntermissionTimerHandle, Del, IntermissionSeconds, false);
	}
	else
	{
		StartWave(NextWave); // 인터미션 0 → 즉시 다음 웨이브.
	}
}

void ACH3TeamProjectGameMode::EnterBossPhase()
{
	bBossPhaseActive = true;
	SpawnSingleEnemy(EEnemyType::Boss);
	UE_LOG(LogCH3GameMode, Log, TEXT("보스전 진입: 보스 소환. 보스를 처치하면 승리."));
	// UI가 '보스전' 연출을 하려면 GetGamePlayState() 유지 + 별도 이벤트 필요 시 여기서 확장.
}

//============================================================================
// 스폰 (플레이어 주변 링)
//============================================================================

void ACH3TeamProjectGameMode::SpawnBatch()
{
	if (CurrentState != EGamePlayState::WaveInProgress)
	{
		return;
	}

	for (int32 i = 0; i < ActiveWaveInfo.SpawnPerBatch; ++i)
	{
		if (EnemiesAlive >= ActiveWaveInfo.MaxConcurrentEnemies)
		{
			break;
		}
		SpawnSingleEnemy(PickWeightedEnemyType(ActiveWaveInfo));
	}
}

bool ACH3TeamProjectGameMode::SpawnSingleEnemy(EEnemyType Type)
{
	const TSubclassOf<AActor>* EnemyClass = EnemyClasses.Find(Type);
	if (!EnemyClass || !(*EnemyClass))
	{
		// AI 몬스터 클래스 미연결: 카운트만 올려 흐름/UI 검증 가능하게 함.
		++EnemiesAlive;
		if (ACH3GameState* GS = GetCH3GameState())
		{
			GS->SetEnemiesRemaining(EnemiesAlive);
		}
		UE_LOG(LogCH3GameMode, Verbose, TEXT("EnemyClasses[type=%d] 미연결. 카운트만 증가(현재 %d)."),
			(int32)Type, EnemiesAlive);
		return false;
	}

	FVector SpawnLoc;
	if (!GetSpawnLocationAroundPlayer(SpawnLoc))
	{
		return false;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	AActor* Spawned = GetWorld()->SpawnActor<AActor>(*EnemyClass, FTransform(FRotator::ZeroRotator, SpawnLoc), Params);
	if (!Spawned)
	{
		return false;
	}

	++EnemiesAlive;
	if (ACH3GameState* GS = GetCH3GameState())
	{
		GS->SetEnemiesRemaining(EnemiesAlive);
	}
	return true;
}

EEnemyType ACH3TeamProjectGameMode::PickWeightedEnemyType(const FWaveInfo& Info) const
{
	int32 Total = 0;
	for (const TPair<EEnemyType, int32>& Pair : Info.SpawnWeights)
	{
		Total += FMath::Max(0, Pair.Value);
	}
	if (Total <= 0)
	{
		return EEnemyType::Normal;
	}

	int32 Roll = FMath::RandRange(1, Total);
	for (const TPair<EEnemyType, int32>& Pair : Info.SpawnWeights)
	{
		Roll -= FMath::Max(0, Pair.Value);
		if (Roll <= 0)
		{
			return Pair.Key;
		}
	}
	return EEnemyType::Normal;
}

bool ACH3TeamProjectGameMode::GetSpawnLocationAroundPlayer(FVector& OutLocation) const
{
	APawn* Player = GetPlayerPawnSafe();
	if (!Player)
	{
		return false;
	}
	const FVector Origin = Player->GetActorLocation();

	// 1) NavMesh 우선: 최대 거리 내 도달 가능 지점, 최소 거리보다 가까우면 재시도.
	if (UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld()))
	{
		const int32 MaxTries = 8;
		for (int32 Try = 0; Try < MaxTries; ++Try)
		{
			FNavLocation NavLoc;
			if (NavSys->GetRandomReachablePointInRadius(Origin, MaxSpawnDistance, NavLoc))
			{
				if (FVector::Dist2D(Origin, NavLoc.Location) >= MinSpawnDistance)
				{
					OutLocation = NavLoc.Location;
					return true;
				}
			}
		}
	}

	// 2) 폴백: 랜덤 각도 + Min~Max 링. 지면에 얹기 위해 아래로 트레이스.
	const float Angle = FMath::FRandRange(0.f, 2.f * PI);
	const float Distance = FMath::FRandRange(MinSpawnDistance, MaxSpawnDistance);
	FVector Candidate = Origin + FVector(FMath::Cos(Angle) * Distance, FMath::Sin(Angle) * Distance, 0.f);

	FHitResult Hit;
	const FVector TraceStart = Candidate + FVector(0.f, 0.f, 500.f);
	const FVector TraceEnd = Candidate - FVector(0.f, 0.f, 2000.f);
	FCollisionQueryParams QParams;
	QParams.AddIgnoredActor(Player);
	if (GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, QParams))
	{
		Candidate.Z = Hit.Location.Z + 50.f;
	}
	OutLocation = Candidate;
	return true;
}

//============================================================================
// 진입점: 다른 파트가 호출
//============================================================================

void ACH3TeamProjectGameMode::NotifyEnemyKilled(EEnemyType EnemyType, AController* Killer)
{
	const int32 Score = GetScoreForEnemy(EnemyType);
	if (ACH3GameState* GS = GetCH3GameState())
	{
		GS->AddScore(Score);
	}

	EnemiesAlive = FMath::Max(0, EnemiesAlive - 1);
	if (ACH3GameState* GS = GetCH3GameState())
	{
		GS->SetEnemiesRemaining(EnemiesAlive);
	}

	UE_LOG(LogCH3GameMode, Verbose, TEXT("적 처치(type=%d), +%d점, 현재 생존 적=%d"),
		(int32)EnemyType, Score, EnemiesAlive);

	// 보스 처치 승리 판정.
	if (bBossPhaseActive && EnemyType == EEnemyType::Boss)
	{
		UE_LOG(LogCH3GameMode, Log, TEXT("보스 처치 → 게임 클리어!"));
		HandleGameClear();
	}
}

void ACH3TeamProjectGameMode::NotifyPlayerDied(AController* PlayerController)
{
	if (CurrentState == EGamePlayState::GameOver || CurrentState == EGamePlayState::GameClear)
	{
		return;
	}
	UE_LOG(LogCH3GameMode, Log, TEXT("플레이어 사망 → 게임 오버."));
	HandleGameOver();
}

void ACH3TeamProjectGameMode::NotifyPlayerLevelUp(AController* PlayerController, int32 NewLevel)
{
	if (CurrentState == EGamePlayState::LevelUpPause)
	{
		UE_LOG(LogCH3GameMode, Warning, TEXT("레벨업 중복(Lv%d). 강화 선택 대기중이라 무시."), NewLevel);
		return;
	}

	PendingUpgradeController = PlayerController;
	CurrentUpgradeCards = RollUpgradeCards();

	StateBeforePause = CurrentState;
	TransitionTo(EGamePlayState::LevelUpPause);

	UGameplayStatics::SetGamePaused(this, true);
	OnUpgradeCardsPresented.Broadcast(CurrentUpgradeCards);

	UE_LOG(LogCH3GameMode, Log, TEXT("레벨업(Lv%d) → 일시정지, 카드 %d장 제시."), NewLevel, CurrentUpgradeCards.Num());
}

void ACH3TeamProjectGameMode::ConfirmUpgradeSelection(EUpgradeType ChosenUpgrade)
{
	if (CurrentState != EGamePlayState::LevelUpPause)
	{
		UE_LOG(LogCH3GameMode, Warning, TEXT("강화 확정 호출됐으나 LevelUpPause 상태 아님."));
		return;
	}
	if (!CurrentUpgradeCards.Contains(ChosenUpgrade))
	{
		UE_LOG(LogCH3GameMode, Warning, TEXT("제시되지 않은 강화 선택 시도. 무시."));
		return;
	}

	OnUpgradeConfirmed.Broadcast(ChosenUpgrade, PendingUpgradeController.Get());

	CurrentUpgradeCards.Reset();
	PendingUpgradeController = nullptr;

	UGameplayStatics::SetGamePaused(this, false);
	TransitionTo(StateBeforePause);

	UE_LOG(LogCH3GameMode, Log, TEXT("강화 선택 완료(type=%d) → 게임 재개."), (int32)ChosenUpgrade);
}

//============================================================================
// 승패
//============================================================================

void ACH3TeamProjectGameMode::HandleGameOver()
{
	GetWorldTimerManager().ClearTimer(IntermissionTimerHandle);
	GetWorldTimerManager().ClearTimer(WaveTimerHandle);
	GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
	bBossPhaseActive = false;
	TransitionTo(EGamePlayState::GameOver);
}

void ACH3TeamProjectGameMode::HandleGameClear()
{
	GetWorldTimerManager().ClearTimer(IntermissionTimerHandle);
	GetWorldTimerManager().ClearTimer(WaveTimerHandle);
	GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
	bBossPhaseActive = false;
	TransitionTo(EGamePlayState::GameClear);
	UE_LOG(LogCH3GameMode, Log, TEXT("게임 클리어!"));
}

//============================================================================
// 디버그/치트
//============================================================================

void ACH3TeamProjectGameMode::WinNow()
{
	UE_LOG(LogCH3GameMode, Log, TEXT("[치트] WinNow"));
	HandleGameClear();
}

void ACH3TeamProjectGameMode::LoseNow()
{
	UE_LOG(LogCH3GameMode, Log, TEXT("[치트] LoseNow"));
	HandleGameOver();
}

void ACH3TeamProjectGameMode::SkipWave()
{
	UE_LOG(LogCH3GameMode, Log, TEXT("[치트] SkipWave (현재 %d)"), ActiveWave);
	if (CurrentState != EGamePlayState::WaveInProgress)
	{
		return;
	}
	GetWorldTimerManager().ClearTimer(WaveTimerHandle);
	if (IsFinalWave(ActiveWave))
	{
		HandleGameClear(); // 디버그 편의: 마지막이면 바로 클리어.
	}
	else
	{
		AdvanceToNextWave();
	}
}

void ACH3TeamProjectGameMode::DebugSetWaveTime(float Seconds)
{
	RemainingWaveTime = FMath::Max(0.f, Seconds);
	if (ACH3GameState* GS = GetCH3GameState())
	{
		GS->SetWaveTimeRemaining(RemainingWaveTime);
	}
	UE_LOG(LogCH3GameMode, Log, TEXT("[치트] SetWaveTime = %.1f"), RemainingWaveTime);
}

void ACH3TeamProjectGameMode::SpawnBossNow()
{
	UE_LOG(LogCH3GameMode, Log, TEXT("[치트] SpawnBoss"));
	EnterBossPhase();
}

void ACH3TeamProjectGameMode::RegisterCheatCommands()
{
	IConsoleManager& CM = IConsoleManager::Get();

	ConsoleCommands.Add(CM.RegisterConsoleCommand(TEXT("CH3.WinNow"), TEXT("[치트] 즉시 게임 클리어"),
		FConsoleCommandDelegate::CreateUObject(this, &ACH3TeamProjectGameMode::WinNow), ECVF_Cheat));

	ConsoleCommands.Add(CM.RegisterConsoleCommand(TEXT("CH3.LoseNow"), TEXT("[치트] 즉시 게임 오버"),
		FConsoleCommandDelegate::CreateUObject(this, &ACH3TeamProjectGameMode::LoseNow), ECVF_Cheat));

	ConsoleCommands.Add(CM.RegisterConsoleCommand(TEXT("CH3.SkipWave"), TEXT("[치트] 현재 웨이브 즉시 넘김"),
		FConsoleCommandDelegate::CreateUObject(this, &ACH3TeamProjectGameMode::SkipWave), ECVF_Cheat));

	ConsoleCommands.Add(CM.RegisterConsoleCommand(TEXT("CH3.SpawnBoss"), TEXT("[치트] 보스 즉시 소환(보스전)"),
		FConsoleCommandDelegate::CreateUObject(this, &ACH3TeamProjectGameMode::SpawnBossNow), ECVF_Cheat));

	ConsoleCommands.Add(CM.RegisterConsoleCommand(TEXT("CH3.SetWaveTime"), TEXT("[치트] 현재 웨이브 남은 시간(초) 설정: CH3.SetWaveTime 10"),
		FConsoleCommandWithArgsDelegate::CreateLambda([this](const TArray<FString>& Args)
		{
			if (Args.Num() > 0)
			{
				DebugSetWaveTime(FCString::Atof(*Args[0]));
			}
		}), ECVF_Cheat));
}

void ACH3TeamProjectGameMode::UnregisterCheatCommands()
{
	IConsoleManager& CM = IConsoleManager::Get();
	for (IConsoleObject* Cmd : ConsoleCommands)
	{
		if (Cmd)
		{
			CM.UnregisterConsoleObject(Cmd);
		}
	}
	ConsoleCommands.Empty();
}

//============================================================================
// 내부 유틸
//============================================================================

void ACH3TeamProjectGameMode::TransitionTo(EGamePlayState NewState)
{
	if (CurrentState == NewState)
	{
		return;
	}
	CurrentState = NewState;

	if (ACH3GameState* GS = GetCH3GameState())
	{
		GS->SetPlayState(NewState);
	}
}

FWaveInfo ACH3TeamProjectGameMode::GetWaveInfo(int32 WaveNumber) const
{
	// 1) 데이터 테이블 우선
	if (WaveTable)
	{
		const FString Context = TEXT("GetWaveInfo");
		const FName RowName = FName(*FString::FromInt(WaveNumber));
		if (const FWaveInfo* Row = WaveTable->FindRow<FWaveInfo>(RowName, Context, false))
		{
			return *Row;
		}
	}

	// 2) 테이블 없거나 행이 없으면 GameMode 노브 값으로 생성.
	FWaveInfo Info;
	Info.WaveNumber = WaveNumber;
	Info.WaveDurationSeconds = WaveDuration;
	Info.SpawnInterval = SpawnInterval;
	Info.SpawnPerBatch = SpawnPerBatch;
	Info.MaxConcurrentEnemies = MaxConcurrentEnemies;
	Info.bIsBossWave = false; // 보스는 WinCondition 로직에서 처리.

	if (bScaleDifficultyByWave)
	{
		const int32 Step = WaveNumber - 1;
		Info.SpawnInterval = FMath::Max(0.3f, SpawnInterval - 0.1f * Step);
		Info.SpawnPerBatch = SpawnPerBatch + Step;
		Info.MaxConcurrentEnemies = MaxConcurrentEnemies + 10 * Step;
	}

	// 종류 구성: 초반 Normal 위주, 후반 Rush/Tanker 비중 증가.
	Info.SpawnWeights.Add(EEnemyType::Normal, 10);
	if (WaveNumber >= 2)
	{
		Info.SpawnWeights.Add(EEnemyType::Rush, 2 * WaveNumber);
	}
	if (WaveNumber >= 3)
	{
		Info.SpawnWeights.Add(EEnemyType::Tanker, WaveNumber);
	}
	return Info;
}

TArray<EUpgradeType> ACH3TeamProjectGameMode::RollUpgradeCards() const
{
	// 힐 포함 전체 풀 (힐도 강화와 동일한 후보 중 하나).
	TArray<EUpgradeType> Pool;
	for (uint8 i = 0; i < (uint8)EUpgradeType::MAX; ++i)
	{
		Pool.Add((EUpgradeType)i);
	}

	// 중복 없이 UpgradeCardCount 장 균등 추첨.
	TArray<EUpgradeType> Result;
	const int32 Num = FMath::Min(UpgradeCardCount, Pool.Num());
	for (int32 i = 0; i < Num; ++i)
	{
		const int32 Index = FMath::RandRange(i, Pool.Num() - 1);
		Pool.Swap(i, Index);
		Result.Add(Pool[i]);
	}
	return Result;
}

int32 ACH3TeamProjectGameMode::GetScoreForEnemy(EEnemyType EnemyType) const
{
	if (const int32* Found = ScorePerEnemyType.Find(EnemyType))
	{
		return *Found;
	}
	return 0;
}
