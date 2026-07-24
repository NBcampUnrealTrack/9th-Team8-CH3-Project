// Copyright Epic Games, Inc. All Rights Reserved.

#include "CH3GameState.h"
#include "Net/UnrealNetwork.h"

ACH3GameState::ACH3GameState()
{
}

void ACH3GameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACH3GameState, PlayState);
	DOREPLIFETIME(ACH3GameState, TotalScore);
		// 추가 : UI담당 - 김민석 : [필수과제] 킬 카운트를 위해 수정.(07/23)
	DOREPLIFETIME(ACH3GameState, TotalKillCount);
	DOREPLIFETIME(ACH3GameState, CurrentWave);
	DOREPLIFETIME(ACH3GameState, TotalWaves);
	DOREPLIFETIME(ACH3GameState, EnemiesRemaining);
	DOREPLIFETIME(ACH3GameState, WaveTimeRemaining);
}

// ==================== Setter (GameMode 권한) ====================
// 스탠드얼론/리슨서버에서는 OnRep이 호출되지 않으므로 여기서 직접 브로드캐스트합니다.

void ACH3GameState::SetPlayState(EGamePlayState NewState)
{
	if (PlayState == NewState)
	{
		return;
	}
	PlayState = NewState;
	OnGamePlayStateChanged.Broadcast(PlayState);
}

void ACH3GameState::SetTotalScore(int32 NewScore)
{
	if (TotalScore == NewScore)
	{
		return;
	}
	TotalScore = NewScore;
	OnScoreChanged.Broadcast(TotalScore);
}

void ACH3GameState::AddScore(int32 Delta)
{
	SetTotalScore(TotalScore + Delta);
}

	// 추가 : UI담당 - 김민석 : [필수과제] 킬 카운트를 위해 수정.(07/23)
void ACH3GameState::SetTotalKillCount(int32 NewCount)
{
	if (TotalKillCount == NewCount)
	{
		return;
	}
	TotalKillCount = NewCount;
	OnKillCountChanged.Broadcast(TotalKillCount);
}

void ACH3GameState::AddKillCount(int32 Delta)
{
	SetTotalKillCount(TotalKillCount + Delta);
}

void ACH3GameState::SetWave(int32 InCurrentWave, int32 InTotalWaves)
{
	CurrentWave = InCurrentWave;
	TotalWaves = InTotalWaves;
	OnWaveChanged.Broadcast(CurrentWave, TotalWaves);
}

void ACH3GameState::SetEnemiesRemaining(int32 Remaining)
{
	const int32 Clamped = FMath::Max(0, Remaining);
	if (EnemiesRemaining == Clamped)
	{
		return;
	}
	EnemiesRemaining = Clamped;
	OnEnemiesRemainingChanged.Broadcast(EnemiesRemaining);
}

void ACH3GameState::SetWaveTimeRemaining(float TimeRemaining)
{
	WaveTimeRemaining = FMath::Max(0.f, TimeRemaining);
	OnWaveTimeChanged.Broadcast(WaveTimeRemaining);
}

// ==================== OnRep (클라이언트) ====================

void ACH3GameState::OnRep_PlayState()
{
	OnGamePlayStateChanged.Broadcast(PlayState);
}

void ACH3GameState::OnRep_TotalScore()
{
	OnScoreChanged.Broadcast(TotalScore);
}

	// 추가 : UI담당 - 김민석 : [필수과제] 킬 카운트를 위해 수정.(07/23)
void ACH3GameState::OnRep_TotalKillCount()
{
	OnKillCountChanged.Broadcast(TotalKillCount);
}

void ACH3GameState::OnRep_Wave()
{
	OnWaveChanged.Broadcast(CurrentWave, TotalWaves);
}

void ACH3GameState::OnRep_EnemiesRemaining()
{
	OnEnemiesRemainingChanged.Broadcast(EnemiesRemaining);
}

void ACH3GameState::OnRep_WaveTime()
{
	OnWaveTimeChanged.Broadcast(WaveTimeRemaining);
}
