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
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<ABaseEnemy> EnemyClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SpawnCount = 1;
};

USTRUCT(BlueprintType)
struct FEnemyWaveInfo
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FEnemySpawnInfo> SpawnInfos;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnEnemyWaveChanged,
	int32, 
	CurrentWave,
	int32, 
	TotalWave
	);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnEnemyCountChanged,
	int32, 
	RemainingEnemyCount
	);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnEnemyWaveCleared,
	int32, 
	ClearedWave
	);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(
	FOnEnemyAllWavesCleared
	);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(
	FOnEnemyBossSpawned
	);

UCLASS()
class CH3TEAMP_API AEnemyWaveManager : public AActor
{
	GENERATED_BODY()
	
public:	
	
	AEnemyWaveManager();

protected:
	
	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	TArray<FEnemyWaveInfo> Waves;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	TArray<AActor*> SpawnPoints;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	float SpawnInterval = 0.5f;
	
	UPROPERTY(BlueprintAssignable, Category = "Wave|UI")
	FOnEnemyWaveChanged OnWaveChanged;
	
	UPROPERTY(BlueprintAssignable, Category = "Wave|UI")
	FOnEnemyCountChanged OnEnemyCountChanged;
	
	UPROPERTY(BlueprintAssignable, Category = "Wave|UI")
	FOnEnemyWaveCleared OnWaveCleared;
	
	UPROPERTY(BlueprintAssignable, Category = "Wave|UI")
	FOnEnemyAllWavesCleared OnAllWavesCleared;
	
	UPROPERTY(BlueprintAssignable, Category = "Wave|UI")
	FOnEnemyBossSpawned OnBossSpawned;

public:
	UFUNCTION(BlueprintCallable, Category = "Wave")
	void StartWave();
	
	UFUNCTION()
	void OnEnemyDied();
	
private:
	void SpawnNextEnemy();
	void FinishWave();
	
private:
	int32 CurrentWaveIndex = 0;
	int32 CurrentSpawnInfoIndex = 0;
	int32 CurrentSpawnedCount = 0;
	int32 RemainingEnemyCount = 0;
	
	FTimerHandle SpawnTimerHandle;
};
