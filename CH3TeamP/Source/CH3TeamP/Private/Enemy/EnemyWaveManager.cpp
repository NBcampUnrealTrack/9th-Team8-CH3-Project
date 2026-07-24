#include "Enemy/EnemyWaveManager.h"
#include "Enemy/BaseEnemy.h"
#include "NavigationSystem.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

AEnemyWaveManager::AEnemyWaveManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AEnemyWaveManager::BeginPlay()
{
	Super::BeginPlay();

	StartWave();
}

void AEnemyWaveManager::StartWave()
{
	if (!Waves.IsValidIndex(CurrentWaveIndex))
	{
		GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
		GetWorldTimerManager().ClearTimer(WaveTimerHandle);

		OnAllWavesCleared.Broadcast();
		return;
	}

	CurrentSpawnInfoIndex = 0;
	CurrentSpawnedCount = 0;
	RemainingEnemyCount = 0;
	AliveEnemies.Empty();

	// 현재 웨이브에서 총 몇 마리가 나와야 하는지 계산
	for (const FEnemySpawnInfo& SpawnInfo : Waves[CurrentWaveIndex].SpawnInfos)
	{
		RemainingEnemyCount += SpawnInfo.SpawnCount;
	}

	OnWaveChanged.Broadcast(CurrentWaveIndex + 1, Waves.Num());
	OnEnemyCountChanged.Broadcast(RemainingEnemyCount);

	// 추가: 웨이브 시간 타이머 시작
	// 몬스터를 다 잡아도 바로 다음 웨이브로 넘어가지 않고,
	// 이 시간이 끝났을 때 EndWave()에서 다음 웨이브를 시작함
	GetWorldTimerManager().ClearTimer(WaveTimerHandle);
	GetWorldTimerManager().SetTimer(
		WaveTimerHandle,
		this,
		&AEnemyWaveManager::EndWave,
		WaveDuration,
		false
	);

	if (RemainingEnemyCount <= 0)
	{
		EndWave();
		return;
	}

	GetWorldTimerManager().SetTimer(
		SpawnTimerHandle,
		this,
		&AEnemyWaveManager::SpawnNextWave,
		SpawnInterval,
		true
	);
}

void AEnemyWaveManager::SpawnNextWave()
{
	if (!Waves.IsValidIndex(CurrentWaveIndex))
	{
		GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
		return;
	}

	FEnemyWaveInfo& CurrentWave = Waves[CurrentWaveIndex];

	if (!CurrentWave.SpawnInfos.IsValidIndex(CurrentSpawnInfoIndex))
	{
		GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
		return;
	}

	FEnemySpawnInfo& CurrentSpawnInfo = CurrentWave.SpawnInfos[CurrentSpawnInfoIndex];

	// 현재 몬스터 종류를 목표 수만큼 다 스폰했으면 다음 몬스터 종류로 넘어감
	if (CurrentSpawnedCount >= CurrentSpawnInfo.SpawnCount)
	{
		CurrentSpawnInfoIndex++;
		CurrentSpawnedCount = 0;
		return;
	}

	if (!CurrentSpawnInfo.EnemyClass)
	{
		CurrentSpawnInfoIndex++;
		CurrentSpawnedCount = 0;
		return;
	}

	FVector SpawnLocation;
	if (!TryGetRandomSpawnLocation(SpawnLocation))
	{
		return;
	}

	const FRotator SpawnRotation = FRotator::ZeroRotator;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;

	// 겹쳐서 태어난 좀비가 밀리거나 맵 아래로 떨어지는 문제를 줄이기 위한 설정
	SpawnParams.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

	ABaseEnemy* SpawnedEnemy = GetWorld()->SpawnActor<ABaseEnemy>(
		CurrentSpawnInfo.EnemyClass,
		SpawnLocation,
		SpawnRotation,
		SpawnParams
	);

	if (!SpawnedEnemy)
	{
		return;
	}

	// 몬스터가 죽었을 때 웨이브 매니저가 남은 적 수를 줄이도록 연결
	SpawnedEnemy->OnEnemyDied.AddDynamic(this, &AEnemyWaveManager::HandleEnemyDied);

	// 추가: 웨이브 종료 시 남아있는 몬스터를 정리하기 위해 저장
	AliveEnemies.Add(SpawnedEnemy);

	CurrentSpawnedCount++;

	if (GEngine)
	{
		const FString DebugText = FString::Printf(
			TEXT("Spawned Enemy: %s / Location: %s"),
			*SpawnedEnemy->GetName(),
			*SpawnLocation.ToString()
		);

	}
}

// 추가: 웨이브 시간이 끝났을 때 호출되는 함수
void AEnemyWaveManager::EndWave()
{
	// 현재 웨이브의 스폰 중단
	GetWorldTimerManager().ClearTimer(SpawnTimerHandle);

	// 추가: 시간이 끝났는데 남아있는 몬스터가 있으면 정리
	// 이전 웨이브 몬스터가 다음 웨이브 카운트를 깎는 문제를 막기 위함
	for (ABaseEnemy* Enemy : AliveEnemies)
	{
		if (IsValid(Enemy))
		{
			Enemy->Destroy();
		}
	}

	AliveEnemies.Empty();

	RemainingEnemyCount = 0;
	OnEnemyCountChanged.Broadcast(RemainingEnemyCount);

	// 다음 웨이브로 이동
	CurrentWaveIndex++;

	// 다음 웨이브 시작
	StartWave();
}

void AEnemyWaveManager::HandleEnemyDied(ABaseEnemy* DeadEnemy)
{
	// 추가: 죽은 몬스터는 살아있는 몬스터 목록에서 제거
	AliveEnemies.Remove(DeadEnemy);

	RemainingEnemyCount = FMath::Max(0, RemainingEnemyCount - 1);
	OnEnemyCountChanged.Broadcast(RemainingEnemyCount);

	// 핵심:
	// 적이 모두 죽어도 여기서 StartWave()를 호출하지 않음.
	// 다음 웨이브 시작은 WaveTimer가 끝났을 때 EndWave()에서 처리함.
	if (RemainingEnemyCount <= 0)
	{
		GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
		return;
	}
}

bool AEnemyWaveManager::TryGetRandomSpawnLocation(FVector& OutLocation) const
{
	if (SpawnPoints.Num() <= 0)
	{
		return false;
	}

	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!NavSystem)
	{
		return false;
	}

	for (int32 TryIndex = 0; TryIndex < SpawnRetryCount; TryIndex++)
	{
		AActor* SpawnPoint = SpawnPoints[FMath::RandRange(0, SpawnPoints.Num() - 1)];
		if (!SpawnPoint)
		{
			continue;
		}

		const FVector RandomOffset = FVector(
			FMath::RandRange(-SpawnRandomRadius, SpawnRandomRadius),
			FMath::RandRange(-SpawnRandomRadius, SpawnRandomRadius),
			SpawnHeightOffset
		);

		const FVector RawLocation = SpawnPoint->GetActorLocation() + RandomOffset;

		FNavLocation ProjectedLocation;
		const bool bFoundNavLocation = NavSystem->ProjectPointToNavigation(
			RawLocation,
			ProjectedLocation,
			NavProjectionExtent
		);

		if (bFoundNavLocation)
		{
			OutLocation = ProjectedLocation.Location + FVector(0.0f, 0.0f, SpawnHeightOffset);
			return true;
		}
	}

	return false;
}