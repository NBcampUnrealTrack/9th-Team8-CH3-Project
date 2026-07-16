#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemyWaveManager.generated.h"

class ABaseEnemy;

USTRUCT(BlueprintType)
struct FEnemySpawnInfo
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<ABaseEnemy> EnemyClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SpawnCount = 1;
};

USTRUCT(BlueprintType)
struct FWaveInfo
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FEnemySpawnInfo> Enemies;
};

UCLASS()
class CH3TEAMP_API AEnemyWaveManager : public AActor
{
	GENERATED_BODY()
	
public:	
	
	AEnemyWaveManager();

protected:
	
	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	TArray<FWaveInfo> Waves;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	TArray<AActor*> SpawnPoints;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	float SpawnInterval = 0.5f;

private:
	int32 CurrentWaveIndex = 0;
	int32 AliveEnemyCount = 0;
	
	FTimerHandle SpawnTimerHandle;
	
	void StartWave();
	void SpawnWaveEnemies();
	void SpawnEnemy(TSubclassOf<ABaseEnemy> EnemyClass);

};
