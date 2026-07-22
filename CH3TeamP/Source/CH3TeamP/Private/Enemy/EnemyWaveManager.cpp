#include "Enemy/EnemyWaveManager.h"
#include "Enemy/BaseEnemy.h"
#include "Engine/World.h"
#include "TimerManager.h"

AEnemyWaveManager::AEnemyWaveManager()
{

	PrimaryActorTick.bCanEverTick = true;

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
		OnAllWavesCleared.Broadcast();
		return;
	}
	
	CurrentSpawnInfoIndex = 0;
	CurrentSpawnedCount = 0;
	RemainingEnemyCount = 0;
	
	for (const FEnemySpawnInfo& SpawnInfo : Waves[CurrentWaveIndex].SpawnInfos)
	{
		RemainingEnemyCount += SpawnInfo.SpawnCount;
	}
	
	OnWaveChanged.Broadcast(CurrentWaveIndex + 1, Waves.Num());
	
	OnEnemyCountChanged.Broadcast(RemainingEnemyCount);
	
	GetWorldTimerManager().SetTimer(
		SpawnTimerHandle,
		this,
		&AEnemyWaveManager::SpawnNextEnemy,
		SpawnInterval,
		true
		);
}

void AEnemyWaveManager::SpawnNextEnemy()
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
	
	if (!CurrentSpawnInfo.EnemyClass)
	{
		CurrentSpawnInfoIndex++;
		CurrentSpawnedCount = 0; 
		return;
	}
	
	if (SpawnPoints.Num() <= 0)
	{
		GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
		return;
	}
	
	const int32 RandomSpawnIndex = FMath::RandRange(0, SpawnPoints.Num() - 1);
	AActor* SpawnPoint = SpawnPoints[RandomSpawnIndex];
	
	if (!SpawnPoint)
	{
		return;
	}
	
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = 
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	
	ABaseEnemy* SpawnedEnemy = GetWorld()->SpawnActor<ABaseEnemy>(
		CurrentSpawnInfo.EnemyClass,
		SpawnPoint->GetActorLocation(),
		SpawnPoint->GetActorRotation(),
		SpawnParams
		);
	
	if (SpawnedEnemy)
	{
		if (SpawnedEnemy->GetClass()->GetName().Contains(TEXT("Boss")))
		{
			OnBossSpawned.Broadcast();
		}
	}
	
	CurrentSpawnedCount++;
	
	if (CurrentSpawnedCount >= CurrentSpawnInfo.SpawnCount)
	{
		CurrentSpawnInfoIndex++;
		CurrentSpawnedCount = 0;
	}
	
	if (CurrentSpawnInfoIndex >= CurrentWave.SpawnInfos.Num())
	{
		GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
	}
}

void AEnemyWaveManager::OnEnemyDied()
{
	RemainingEnemyCount--;
	
	if (RemainingEnemyCount < 0)
	{
		RemainingEnemyCount = 0;
	}
	
	OnEnemyCountChanged.Broadcast(RemainingEnemyCount);
	
	if (RemainingEnemyCount <= 0)
	{
		FinishWave();
	}
}

void AEnemyWaveManager::FinishWave()
{
	OnWaveCleared.Broadcast(CurrentWaveIndex + 1);
	
	CurrentWaveIndex++;
	
	if (!Waves.IsValidIndex(CurrentWaveIndex))
	{
		OnAllWavesCleared.Broadcast();
		return;
	}
	
	StartWave();
}