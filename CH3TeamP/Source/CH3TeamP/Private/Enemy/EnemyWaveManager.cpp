#include "Enemy/EnemyWaveManager.h"
#include "Enemy/BaseEnemy.h"
#include "Engine/World.h"

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
		return;
	}
	
	SpawnWaveEnemies();
}

void AEnemyWaveManager::SpawnWaveEnemies()
{
	const FWaveInfo& CurrentWave = Waves[CurrentWaveIndex];
	
	for (const FEnemySpawnInfo& SpawnInfo : CurrentWave.Enemies)
	{
		for (int32 i = 0; i < SpawnInfo.SpawnCount; i++)
		{
			SpawnEnemy(SpawnInfo.EnemyClass);
		}
	}
}

void AEnemyWaveManager::SpawnEnemy(TSubclassOf<ABaseEnemy> EnemyClass)
{
	if (!EnemyClass || SpawnPoints.Num() == 0)
	{
		return;
	}
	
	const int32 RandomIndex = FMath::RandRange(0, SpawnPoints.Num() - 1);
	AActor* SpawnPoint = SpawnPoints[RandomIndex];
	
	if (!SpawnPoint)
	{
		return;
	}
	
	FActorSpawnParameters SpawnParams;
	SpawnParams. SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	
	ABaseEnemy* SpawnedEnemy = GetWorld()->SpawnActor<ABaseEnemy>(
		EnemyClass,
		SpawnPoint->GetActorLocation(),
		SpawnPoint->GetActorRotation(),
		SpawnParams
		);
	
	if (SpawnedEnemy)
	{
		AliveEnemyCount++;
	}
}