// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CH3MainHUDWidget.generated.h"

/**
 * 
 */
UCLASS()
class CH3TEAMP_API UCH3MainHUDWidget : public UUserWidget
{
	GENERATED_BODY()
	
	
	
	
	// 미니맵 관련 부분.
protected:
	UPROPERTY(meta = (BindWidget))
	class UImage* MinimapBackgroundImage;

	UPROPERTY(meta = (BindWidget))
	class UCanvasPanel* MinimapBlipCanvas;

	// 아까 계산하신 좌표값을 기본값으로 넣어두기
	UPROPERTY(EditAnywhere, Category = "Minimap")
	FVector2D MinimapWorldOrigin = FVector2D(-154064.288f, -83505.0f);

	UPROPERTY(EditAnywhere, Category = "Minimap")
	FVector2D MinimapWorldSize = FVector2D(300000.0f, 168750.0f);

	UPROPERTY(EditAnywhere, Category = "Minimap")
	FVector2D MinimapImagePixelSize = FVector2D(300.f, 300.f); // UMG에서 배치할 크기와 일치시킬 것

	UPROPERTY(EditAnywhere, Category = "Minimap")
	TSubclassOf<class UMiniMapWidget> PlayerBlipWidgetClass;

	UPROPERTY()
	class UMiniMapWidget* PlayerBlip;

	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float DeltaTime) override;

	FVector2D WorldToMinimapPosition(const FVector& WorldLocation);
};



/*HUD에 구현할 것들. 메인메뉴 : MainMenu
1. 스테이터스
	1-1) 체력바
	1-2) 스태미나(스태미나바)
		(1) 스프린트(달리기)와 연결.
	1-3) 경험치.
	
2. 자원정보 표시
	2-1) 무기 변경 아이콘
		weapon 관련 클래스에서
	2-2) 탄창 (잔여갯수 / 전체 갯수)
	2-3) 상태이상(탈진(스태미나 = 0) 등등)
	2-4) 웨이브 사이 대기 시간(초). 라운드 별.
3. 미션 정보
	3-1) 미션 목표
		! 목표가 시간인지, 몬스터 사냥인지 확인. 보스 등장 조건도 확인할 것. !
		(1) 시간
			-> 진행 시간(ActiveWave)(혹은 남은 시간 : DebugSetWaveTime) / 웨이브 시간(WaveDuration)
			-> 보스전 처치 (보스 처치 수 / 보스 수(1))
		(2) 몬스터 사냥
			-> 몬스터 사냥은 포인트로만.
	3-2) 진행상황
		몬스터 종류별 처치 점수 가져오기 : ScorePerEnemyType;
4. 미니맵(가능할까?)


ㅡㅡㅡㅡㅡㅡ

다른 곳에서 구현할 것.
1. 세부 스테이터스 창.
	1-1) 기본 능력치
		(1) 현재 체력 / 최대 체력
		(2) 현재 스태미나 / 최대 스태미나
		(3) 현재 경험치 / 최대 경험치
	1-2) 추가 스탯
		class EUpgradeType
			AttackUp		UMETA(DisplayName = "공격력 증가"),
			FireRateUp		UMETA(DisplayName = "연사 속도 증가"),
			MoveSpeedUp		UMETA(DisplayName = "이동 속도 증가"),
			MaxHealthUp		UMETA(DisplayName = "최대 체력 증가"),
			CritChanceUp	UMETA(DisplayName = "치명타 확률 증가"),
			MagazineUp		UMETA(DisplayName = "탄창 증가"),
			ExplosiveAmmo	UMETA(DisplayName = "폭발탄 획득"),
			PiercingAmmo	UMETA(DisplayName = "관통탄 획득"),
			StaminaUp		UMETA(DisplayName = "스태미나 증가"),
			ReloadSpeedUp	UMETA(DisplayName = "장전 속도 증가"),
			Heal			UMETA(DisplayName = "회복(힐 카드)"),

			MAX				UMETA(Hidden)
2. 레벨업 시
	2-1) 레벨업/강화 선택(일시정지) : LevelUpPause
	2-2) 
	

*/