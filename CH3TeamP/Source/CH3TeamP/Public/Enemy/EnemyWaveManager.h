#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemyWaveManager.generated.h"

class ABaseEnemy;

USTRUCT(BlueprintType)
struct FEnemySpawnInfo
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	TSubclassOf<ABaseEnemy> EnemyClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	int32 SpawnCount = 0;
};

USTRUCT(BlueprintType)
struct FEnemyWaveInfo
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	TArray<FEnemySpawnInfo> SpawnInfos;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEnemyWaveChangedSignature, int32, CurrentWave, int32, TotalWave);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEnemyCountChangedSignature, int32, RemainingEnemyCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAllEnemyWavesClearedSignature);

UCLASS()
class CH3TEAMP_API AEnemyWaveManager : public AActor
{
	GENERATED_BODY()

public:
	AEnemyWaveManager();

	UPROPERTY(BlueprintAssignable, Category = "Wave")
	FEnemyWaveChangedSignature OnWaveChanged;

	UPROPERTY(BlueprintAssignable, Category = "Wave")
	FEnemyCountChangedSignature OnEnemyCountChanged;

	UPROPERTY(BlueprintAssignable, Category = "Wave")
	FAllEnemyWavesClearedSignature OnAllWavesCleared;

protected:
	virtual void BeginPlay() override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	TArray<FEnemyWaveInfo> Waves;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Points")
	TArray<AActor*> SpawnPoints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Points")
	float SpawnInterval = 0.5f;

	// 추가: 한 웨이브가 유지되는 시간
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	float WaveDuration = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Points")
	float SpawnRandomRadius = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Points")
	float SpawnHeightOffset = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Points")
	int32 SpawnRetryCount = 20;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Points")
	FVector NavProjectionExtent = FVector(500.0f, 500.0f, 300.0f);

	FTimerHandle SpawnTimerHandle;

	// 추가: 웨이브 시간이 끝났을 때 다음 웨이브로 넘기기 위한 타이머
	FTimerHandle WaveTimerHandle;

	int32 CurrentWaveIndex = 0;
	int32 CurrentSpawnInfoIndex = 0;
	int32 CurrentSpawnedCount = 0;
	int32 RemainingEnemyCount = 0;

	// 추가: 현재 웨이브에서 살아있는 몬스터 추적
	UPROPERTY()
	TArray<TObjectPtr<ABaseEnemy>> AliveEnemies;

	void StartWave();
	void SpawnNextWave();

	// 추가: 웨이브 시간이 끝났을 때 호출됨
	void EndWave();

	UFUNCTION()
	void HandleEnemyDied(ABaseEnemy* DeadEnemy);

	bool TryGetRandomSpawnLocation(FVector& OutLocation) const;
};