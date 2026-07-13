// Copyright Epic Games, Inc. All Rights Reserved.
// 팀 공용 게임플레이 타입 정의 (담당: 김유탁 / 게임모드)
// 다른 파트(전투/AI/UI)에서도 이 헤더를 include 하여 동일한 타입을 공유합니다.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "CH3GameplayTypes.generated.h"

/** 전체 게임 진행 상태 */
UENUM(BlueprintType)
enum class EGamePlayState : uint8
{
	MainMenu			UMETA(DisplayName = "메인메뉴"),
	Starting			UMETA(DisplayName = "게임시작/초기화"),
	WaveInProgress		UMETA(DisplayName = "웨이브 진행중"),
	WaveIntermission	UMETA(DisplayName = "웨이브 종료/대기"),
	LevelUpPause		UMETA(DisplayName = "레벨업/강화 선택(일시정지)"),
	GameClear			UMETA(DisplayName = "게임 클리어"),
	GameOver			UMETA(DisplayName = "게임 오버")
};

/** 몬스터 종류. AI(성태현)와 식별 규격을 통일하기 위한 enum. */
UENUM(BlueprintType)
enum class EEnemyType : uint8
{
	Normal		UMETA(DisplayName = "일반 좀비"),
	Rush		UMETA(DisplayName = "돌격형"),
	Tanker		UMETA(DisplayName = "탱커형"),
	Boss		UMETA(DisplayName = "보스"),

	MAX			UMETA(Hidden)
};

/** 레벨업 시 제시되는 강화(및 힐) 종류. 실제 효과 적용은 전투 로직 담당. */
UENUM(BlueprintType)
enum class EUpgradeType : uint8
{
	AttackUp		UMETA(DisplayName = "공격력 증가"),
	FireRateUp		UMETA(DisplayName = "연사 속도 증가"),
	MoveSpeedUp		UMETA(DisplayName = "이동 속도 증가"),
	MagazineUp		UMETA(DisplayName = "탄창 증가"),
	ExplosiveAmmo	UMETA(DisplayName = "폭발탄 획득"),
	PiercingAmmo	UMETA(DisplayName = "관통탄 획득"),
	StaminaUp		UMETA(DisplayName = "스태미나 증가"),
	ReloadSpeedUp	UMETA(DisplayName = "장전 속도 증가"),
	Heal			UMETA(DisplayName = "회복(힐 카드)"),

	MAX				UMETA(Hidden)
};

/** 게임 승리(클리어) 조건. 에디터/설정에서 손쉽게 전환. */
UENUM(BlueprintType)
enum class EGameWinCondition : uint8
{
	/** 모든 웨이브를 시간까지 버티면 승리 (보스 없음). 기본. */
	SurviveAllWaves			UMETA(DisplayName = "타이머 생존 승리"),
	/** 마지막 웨이브 시작 시 보스 등장 → 보스를 처치해야 승리. */
	DefeatBoss				UMETA(DisplayName = "보스 처치 승리"),
	/** 마지막 웨이브까지 생존한 뒤 보스 등장 → 보스 처치 시 승리. */
	SurviveThenDefeatBoss	UMETA(DisplayName = "생존 후 보스 처치")
};

/**
 * 웨이브 1개의 구성 데이터 (생존/연속 스폰 방식).
 * 각 웨이브는 WaveDurationSeconds 동안 지속되며, 그 시간 동안 플레이어 주변에
 * 몬스터가 계속 스폰됩니다. 시간을 버티면(생존) 웨이브 클리어.
 * DataTable Row 로 사용하여 코드 수정 없이 밸런싱합니다.
 */
USTRUCT(BlueprintType)
struct FWaveInfo : public FTableRowBase
{
	GENERATED_BODY()

	/** 웨이브 번호 (1부터 시작) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	int32 WaveNumber = 1;

	/** 웨이브 지속 시간(초). 기본 120초(2분) 생존. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	float WaveDurationSeconds = 120.f;

	/** 스폰 배치 간격(초). 이 간격마다 SpawnPerBatch 만큼 소환. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave", meta = (ClampMin = "0.1"))
	float SpawnInterval = 2.f;

	/** 한 번에 소환할 몬스터 수. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave", meta = (ClampMin = "1"))
	int32 SpawnPerBatch = 3;

	/** 동시에 존재할 수 있는 최대 몬스터 수 (성능/난이도 상한). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave", meta = (ClampMin = "1"))
	int32 MaxConcurrentEnemies = 30;

	/** 종류별 스폰 가중치(상대 확률). 예) Normal=7, Rush=2, Tanker=1 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	TMap<EEnemyType, int32> SpawnWeights;

	/** 보스 웨이브 여부. true면 시작 시 보스 1마리를 즉시 소환. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	bool bIsBossWave = false;
};
