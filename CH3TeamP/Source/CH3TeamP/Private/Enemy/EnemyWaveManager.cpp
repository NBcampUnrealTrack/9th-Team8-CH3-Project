#include "Enemy/EnemyWaveManager.h"
#include "Enemy/BaseEnemy.h"

#include "NavigationSystem.h"
#include "TimerManager.h"
#include "Engine/World.h"


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
	// 모든 웨이브가 종료됐는지 확인
	if (!Waves.IsValidIndex(CurrentWaveIndex))
	{
		GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
		GetWorldTimerManager().ClearTimer(WaveTimerHandle);

		OnAllWavesCleared.Broadcast();
		return;
	}

	// 새로운 웨이브의 상태 초기화
	CurrentSpawnInfoIndex = 0;
	CurrentSpawnedCount = 0;
	RemainingEnemyCount = 0;
	AliveEnemies.Empty();

	// 현재 웨이브에서 생성할 전체 적의 수 계산
	for (const FEnemySpawnInfo& SpawnInfo :
		Waves[CurrentWaveIndex].SpawnInfos)
	{
		RemainingEnemyCount += SpawnInfo.SpawnCount;
	}

	// UI 등에 웨이브와 적 숫자 변경 통지
	OnWaveChanged.Broadcast(
		CurrentWaveIndex + 1,
		Waves.Num()
	);

	OnEnemyCountChanged.Broadcast(RemainingEnemyCount);

	/*
	 * 웨이브 제한 시간 시작
	 *
	 * 적을 모두 처치해도 즉시 다음 웨이브로 이동하지 않고,
	 * WaveDuration이 끝나면 EndWave()에서 다음 웨이브를 시작합니다.
	 */
	GetWorldTimerManager().ClearTimer(WaveTimerHandle);

	GetWorldTimerManager().SetTimer(
		WaveTimerHandle,
		this,
		&AEnemyWaveManager::EndWave,
		WaveDuration,
		false
	);

	// 생성할 적이 없는 웨이브는 바로 종료
	if (RemainingEnemyCount <= 0)
	{
		EndWave();
		return;
	}

	// 일정 간격으로 적 생성
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

	FEnemyWaveInfo& CurrentWave =
		Waves[CurrentWaveIndex];

	// 현재 웨이브의 모든 적 종류를 생성했는지 확인
	if (!CurrentWave.SpawnInfos.IsValidIndex(
		CurrentSpawnInfoIndex
	))
	{
		GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
		return;
	}

	FEnemySpawnInfo& CurrentSpawnInfo =
		CurrentWave.SpawnInfos[CurrentSpawnInfoIndex];

	// 현재 종류의 적을 모두 생성했다면 다음 종류로 이동
	if (CurrentSpawnedCount >= CurrentSpawnInfo.SpawnCount)
	{
		CurrentSpawnInfoIndex++;
		CurrentSpawnedCount = 0;
		return;
	}

	// 적 클래스가 설정되지 않았다면 다음 종류로 이동
	if (!CurrentSpawnInfo.EnemyClass)
	{
		CurrentSpawnInfoIndex++;
		CurrentSpawnedCount = 0;
		return;
	}

	// NavMesh 위에서 생성 가능한 위치 검색
	FVector SpawnLocation;

	if (!TryGetRandomSpawnLocation(SpawnLocation))
	{
		return;
	}

	const FRotator SpawnRotation =
		FRotator::ZeroRotator;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;

	/*
	 * 충돌하는 위치라면 주변의 생성 가능한 위치로 조정합니다.
	 * 조정할 수 없다면 해당 생성 요청은 실패합니다.
	 */
	SpawnParams.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::
		AdjustIfPossibleButDontSpawnIfColliding;

	ABaseEnemy* SpawnedEnemy =
		GetWorld()->SpawnActor<ABaseEnemy>(
			CurrentSpawnInfo.EnemyClass,
			SpawnLocation,
			SpawnRotation,
			SpawnParams
		);

	if (!IsValid(SpawnedEnemy))
	{
		return;
	}

	// 적이 죽었을 때 WaveManager가 알 수 있도록 이벤트 연결
	SpawnedEnemy->OnEnemyDied.AddDynamic(
		this,
		&AEnemyWaveManager::HandleEnemyDied
	);

	// 현재 살아 있는 적 목록에 추가
	AliveEnemies.Add(SpawnedEnemy);

	CurrentSpawnedCount++;
}


void AEnemyWaveManager::EndWave()
{
	// 현재 웨이브의 추가 적 생성을 중단
	GetWorldTimerManager().ClearTimer(SpawnTimerHandle);

	/*
	 * 제한 시간이 끝났는데 살아 있는 적이 있다면 제거합니다.
	 * 이전 웨이브의 적이 다음 웨이브에 남는 것을 방지합니다.
	 */
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


void AEnemyWaveManager::HandleEnemyDied(
	ABaseEnemy* DeadEnemy
)
{
	// 죽은 적을 생존 목록에서 제거
	AliveEnemies.Remove(DeadEnemy);

	RemainingEnemyCount =
		FMath::Max(0, RemainingEnemyCount - 1);

	OnEnemyCountChanged.Broadcast(RemainingEnemyCount);

	/*
	 * 적을 전부 처치해도 즉시 다음 웨이브를 시작하지 않습니다.
	 * 남은 적 생성만 중단하고 WaveTimer가 끝날 때까지 기다립니다.
	 */
	if (RemainingEnemyCount <= 0)
	{
		GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
	}
}


bool AEnemyWaveManager::TryGetRandomSpawnLocation(
	FVector& OutLocation
) const
{
	if (SpawnPoints.IsEmpty())
	{
		return false;
	}

	UNavigationSystemV1* NavSystem =
		UNavigationSystemV1::GetCurrent(GetWorld());

	if (!IsValid(NavSystem))
	{
		return false;
	}

	// 설정된 횟수만큼 생성 가능한 NavMesh 위치 검색
	for (int32 TryIndex = 0;
		TryIndex < SpawnRetryCount;
		TryIndex++)
	{
		AActor* SpawnPoint =
			SpawnPoints[
				FMath::RandRange(
					0,
					SpawnPoints.Num() - 1
				)
			];

		if (!IsValid(SpawnPoint))
		{
			continue;
		}

		const FVector RandomOffset(
			FMath::RandRange(
				-SpawnRandomRadius,
				SpawnRandomRadius
			),
			FMath::RandRange(
				-SpawnRandomRadius,
				SpawnRandomRadius
			),
			SpawnHeightOffset
		);

		const FVector RawLocation =
			SpawnPoint->GetActorLocation() + RandomOffset;

		FNavLocation ProjectedLocation;

		const bool bFoundNavLocation =
			NavSystem->ProjectPointToNavigation(
				RawLocation,
				ProjectedLocation,
				NavProjectionExtent
			);

		if (bFoundNavLocation)
		{
			/*
			 * NavMesh 표면보다 캐릭터 캡슐이 약간 높은 위치에서
			 * 생성되도록 Z 높이를 추가합니다.
			 */
			OutLocation =
				ProjectedLocation.Location +
				FVector(
					0.0f,
					0.0f,
					SpawnHeightOffset
				);

			return true;
		}
	}

	return false;
}