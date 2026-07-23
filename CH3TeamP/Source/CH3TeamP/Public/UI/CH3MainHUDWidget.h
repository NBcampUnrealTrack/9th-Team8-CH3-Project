// 메인 위젯.
// 플레이어 화면에 계속 떠 있는 HUD 전체.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CH3GameplayTypes.h"
#include "CH3MainHUDWidget.generated.h"


	// 위젯 내에 넣는 것들.
class UProgressBar;
class UTextBlock;
class UImage;
class UCanvasPanel;
class UVerticalBox;


	// 미니맵 마커 및 대미지량, 킬, 미니맵 캡쳐 등 액터.
class UCH3MinimapMarkerWidget;
class UCH3DamageNumberWidget;
class UCH3KillFeedEntryWidget;
class ACH3MinimapCaptureActor;
class ABaseEnemy;	//적 몬스터 위치 확인용
	// 이후 필요한 클래스들 여기에 추가.

	//포인터로만 들고 있기에, 전방 선언.
class APlayerCharacter;

	// 게임 결과 위젯.
class UCH3ResultWidget;


UCLASS()
class CH3TEAMP_API UCH3MainHUDWidget : public UUserWidget
{
	GENERATED_BODY()
	
	
	//플레이어 캐릭터 HUD 부분.(체력 등)
public:
	// Handle 함수들: 팀원의 델리게이트가 Broadcast하면 자동으로 호출됨.
	// UFUNCTION() 매크로 필수 - AddDynamic 바인딩은 리플렉션을 쓰기 때문에.
	
	//GameState에서 가져온 것들.
		// 게임 상태 변경 : 레벨업 UI 표시/숨김/ 웨이브 진행/레벨업/클리어/게임오버 시 - 화면 전환.
		// GameState의 FOnGamePlayStateChanged.
	UFUNCTION() void HandleGamePlayStateChanged(EGamePlayState NewState);

		// 점수 변경 : 점수 텍스트 갱신.
	UFUNCTION() void HandleScoreChanged(int32 NewScore);

		// 웨이브 변경 : Wave가 몇 웨이브인지 표시(GameState)
	UFUNCTION() void HandleWaveChanged(int32 CurrentWave, int32 TotalWaves);

		// 남은 적 수 변경 : 킬카운트 대신.(GameState에 FOnEnemiesRemainingChanged(남은 적) 연결)
	UFUNCTION() void HandleEnemiesRemainingChanged(int32 Remaining);

		// 웨이브 남은 시간 (0.1초마다 갱신)
	UFUNCTION() void HandleWaveTimeChanged(float TimeRemaining);
	
	
	
		// 저체력 붉은 오버레이. WBP의 LowHealthVignette와 이름으로 연결.
		// BindWidgetOptional: WBP에 없어도 컴파일 에러 대신 그냥 동작 안 함(안전).
	UPROPERTY(meta = (BindWidgetOptional))
	class UImage* LowHealthVignette;

		// 이 비율 이하부터 붉은 연출 시작 (0.20 = 20%). 에디터에서 조절 가능하게 노출.
	UPROPERTY(EditDefaultsOnly, Category = "CH3|LowHealth")
	float LowHealthThreshold = 0.20f;

		// 이 비율에서 최대 강도 + 시야 최대 축소 (0.10 = 10%).
	UPROPERTY(EditDefaultsOnly, Category = "CH3|LowHealth")
	float CriticalHealthThreshold = 0.10f;

		// 심장박동 맥동 속도. 클수록 빨리 뜀.
	UPROPERTY(EditDefaultsOnly, Category = "CH3|LowHealth")
	float PulseSpeed = 4.0f;
	
	
	
	
		//======= 맵 경계 경고 오버레이. WBP의 같은 이름 Image와 연결. 평소엔 숨김.
	UPROPERTY(meta = (BindWidgetOptional))
	class UImage* BoundaryWarningVignette;

		// 안전 구역 사각형. 팀원이 배치한 차단 액터 위치 기준으로 측정한 값. <- 취소. 레벨이 기울어짐.
		// (실제 벽 위치와 어긋나면 여기 숫자만 조정하면 됨 — 코드 구조는 안 바뀜)
		// 안전 구역 원형 경계. 레벨이 회전되어 있어 사각형보다 원형이 적합. 은 무슨, 그냥 사각형 하고 각도 조절하기.
	UPROPERTY(EditDefaultsOnly, Category = "CH3|Boundary")
	FVector2D BoundaryCenter = FVector2D(89.58f, -155.22f);

	// 레벨 바닥 액터의 Yaw 회전값 그대로.
	UPROPERTY(EditDefaultsOnly, Category = "CH3|Boundary")
	float BoundaryRotationYaw = 30.0f;
	
	// 회전 안 된 상태 기준 가로/세로 절반 길이.
	UPROPERTY(EditDefaultsOnly, Category = "CH3|Boundary")
	float BoundaryHalfWidth = 6000.f;

	UPROPERTY(EditDefaultsOnly, Category = "CH3|Boundary")
	float BoundaryHalfHeight = 6000.f;

	UPROPERTY(EditDefaultsOnly, Category = "CH3|Boundary")
	float BoundaryWarningMargin = 500.f;
	
	// 벽에 붙었을 때의 최대 어둡기. 1.0(완전 검정)이면 앞이 안 보여서
	// 벽에 막힌 채 돌아갈 방향을 못 찾게 됨. 상한을 둬서 시야를 남긴다.
	// 이 거리(cm)만큼 안쪽부터 경고가 서서히 시작됨. 벽에 닿기 전에 미리 알리기 위함.
	UPROPERTY(EditDefaultsOnly, Category = "CH3|Boundary", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float BoundaryWarningMaxAlpha = 0.8f;

	
	
	
	
	/* 아직 넣지 않을 부분
	//------GameState지만, 넣을지 미확정 부분-----------------
 
		// 킬 카운트 변경 : 킬 수 텍스트 갱신. 소유자: GameState
	UFUNCTION() void HandleKillCountChanged(int32 NewKillCount);
	
		// 탄약 변경(발사/재장전). 탄약 시스템이 아직 없어 스펙만 확정해둠
	UFUNCTION() void HandleAmmoChanged(int32 CurrentAmmo, int32 MaxAmmo);

		// 내 공격 명중 → 히트마커. FireGun()의 명중 분기에서 Broadcast 예정
	UFUNCTION() void HandleHitConfirmed(bool bIsCritical);
	
		// 적 피격 → 그 위치에 데미지 숫자 스폰
	UFUNCTION() void HandleDamageDealt(float DamageAmount, FVector HitLocation, bool bIsCritical);

		
	*/
	
	
protected:
	virtual void NativeConstruct() override;
		//Tick
	virtual void NativeTick(const FGeometry& MyGeometry, float DeltaTime) override;
	
	// 체력/스태미나를 매 프레임 읽어옴
	void PollPlayerStatus(float DeltaTime);
	
		//  GameState 델리게이트에 Handle 함수 연결 + 현재값으로 초기 동기화
	void BindDelegates();

		// 남은시간/남은적을 합쳐 Quest_details 한 줄로 갱신 (두 이벤트가 따로 오므로 캐시 후 조합)
	void RefreshQuestDetails();

	
public:
		// 체력 변경 : 체력바/텍스트 갱신. 소유자: PlayerCharacter
	UFUNCTION() void HandleHealthChanged(float CurrentHealth, float MaxHealth);
	
		//체력바
	UPROPERTY(meta = (BindWidgetOptional))
	UProgressBar* HealthBar;
 
		// 스태미나 변경(달리기 소모 등) : 스태미나바 갱신. 소유자: PlayerCharacter
	UFUNCTION() void HandleStaminaChanged(float CurrentStamina, float MaxStamina);
	
		// 스태미나바
	UPROPERTY(meta = (BindWidgetOptional))
	UProgressBar* StaminaBar;
	
	
		// 경험치 변경 : 경험치바 갱신. 소유자: PlayerCharacter
	UFUNCTION() void HandleEXPChanged(float NewCurrentEXP, float NewMaxEXP);

		// 경험치바. WBP의 EXPbar와 이름 일치 필요.
	UPROPERTY(meta = (BindWidgetOptional))
	UProgressBar* EXPBar;

		// 레벨업 : 레벨 텍스트 갱신. 소유자: PlayerCharacter
	UFUNCTION() void HandleLevelUp(int32 NewLevel);

		// 레벨 텍스트. WBP에 있으면 이름 맞춰 연결(없으면 null로 안전하게 무시됨).
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* LevelText;
	
 
		// 탄약 변경(발사/재장전) : 탄약 텍스트 갱신. 소유자: (확인필요)PlayerCharacter
	UFUNCTION() void HandleAmmoChanged(int32 CurrentAmmo, int32 MaxAmmo);
 
		// 탄환 종류 적용 : 3슬롯 캐러셀 아이콘 갱신. 소유자: (확인필요)PlayerCharacter
	UFUNCTION() void HandleBulletSpeciesChanged(int32 WeaponIndex);
 
		// 내 공격이 적에게 명중 : 히트마커 표시. 소유자: (확인필요)PlayerCharacter
	UFUNCTION() void HandleHitConfirmed(bool bIsCritical);
 
		// 적이 데미지를 입음 : 그 위치에 데미지 숫자 스폰. 소유자: GameState(중계)
	UFUNCTION() void HandleDamageDealt(float DamageAmount, FVector HitLocation, bool bIsCritical);
 
		//총알 갯수.
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* AmmoText;
	
	
		// 현재 탄종 아이콘. 총알 수 아래에 표시.
	UPROPERTY(meta = (BindWidgetOptional))
	UImage* BulletTypeIcon;

		// 탄종별 텍스처. WBP Class Defaults에서 지정.
	UPROPERTY(EditDefaultsOnly, Category = "CH3|Ammo")
	UTexture2D* NormalAmmoTexture;

	UPROPERTY(EditDefaultsOnly, Category = "CH3|Ammo")
	UTexture2D* PiercingAmmoTexture;

	UPROPERTY(EditDefaultsOnly, Category = "CH3|Ammo")
	UTexture2D* ExplosiveAmmoTexture;
	
	
		// 정조준 시 스코프 오버레이. 평소엔 숨김.
	UPROPERTY(meta = (BindWidgetOptional))
	UImage* ScopeOverlay;
	
	
	
		
	
	
private:
		// 폴링 대상 캐릭터 캐시.
		// 매 프레임 캐릭터의 Cast를 반복하지 않도록 한 번 찾으면 저장해둔다.
		// (HUD가 캐릭터보다 먼저 생길 수 있어 NativeTick에서 지연 획득)
		// 캐릭터가 다른 곳에서 소명됐는데, cachedPlayer가 여전히 그 죽은 메모리 주소를 들고 있으면 크래시 발생할 수도 있음.
	UPROPERTY()
	APlayerCharacter* CachedPlayer;
	
		//총알 UI 테스트용
	int32 TestAmmo = 30;

 
		// 탄약 폴링용 캐시.
		// 이유: 탄약은 발사/장전 때만 바뀌는데 매 프레임 SetText를 부르면 낭비.
		// 직전 값과 다를 때만 HandleAmmoChanged를 호출.
		// 실제 탄약은 0 이상이니, 첫 프레임에는 무조건 한 번 갱신되기 위해 -1로 초기화.
	int32 LastPolledAmmo = -1;
	int32 LastPolledMaxAmmo = -1;
	
	
		// 저체력 연출 갱신. 매 프레임 폴링에서 호출.
	void UpdateLowHealthEffect(float HealthPercent, float DeltaTime);

		// 맥동 애니메이션용 누적 시간. 프레임을 넘어 유지돼야 해서 멤버로 둠.
	float PulseAccumulator = 0.0f;
	
	
		// 맵 경계 이탈 경고 갱신. 매 프레임 폴링에서 호출. (저체력 연출과 같은 패턴)
	void UpdateBoundaryWarning(const FVector& PlayerLocation);
	
 
 /* 테스트 함수(캐릭터 연결 뒤) 만약 연결이 안 된 코드는 이걸로 대체하기 위해.
		// 테스트 함수: 연결된 코드가 없어도 UI 동작을 눈으로 확인하는 용도
		// (다른 클래스에서 델리게이트 연결 후에는 삭제 예정)
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Test")
	void Test_SimulateAll();
	*/
	
	
	
	
	
	
	// 미니맵 관련 부분.
protected:
	// 미니맵 좌표를 CaptureActor에서 자동 수신
	void SyncMinimapRangeFromCaptureActor();
	
	
	
	UPROPERTY(meta = (BindWidget))
	class UImage* MinimapBackgroundImage;

	UPROPERTY(meta = (BindWidget))
	class UCanvasPanel* MinimapMarkerCanvas;

		// 계산한 좌표값을 기본값으로 넣어두기
		// 폴백값. 레벨에 CaptureActor가 있으면 NativeConstruct에서 자동 교체됨
	UPROPERTY(EditAnywhere, Category = "Minimap")
	FVector2D MinimapWorldSize = FVector2D(8000.f, 8000.f);
 
	UPROPERTY(EditAnywhere, Category = "Minimap")
	FVector2D MinimapImagePixelSize = FVector2D(300.f, 300.f);
 
	UPROPERTY(EditAnywhere, Category = "Minimap")
	TSubclassOf<UCH3MinimapMarkerWidget> PlayerMarkerWidgetClass;
 
	UPROPERTY()
	UCH3MinimapMarkerWidget* PlayerMarker;
 
	FVector2D WorldToMinimapPosition(const FVector& WorldLocation);
	
	
	
		// 미션 정보 표시용 위젯 (WBP_HUD에 이미 배치된 이름과 일치)
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Quest;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Quest_Timedetails;
	
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Quest_Mobdetails;

		// 점수 표시 (아직 WBP에 없어도 되는 선택적 위젯)
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* ScoreText;
	
	
		// 킬 카운트 변경 : 누적 처치 수 텍스트 갱신. 소유자: GameState
	UFUNCTION() void HandleKillCountChanged(int32 NewKillCount);

		// 누적 처치 수 표시. WBP에 있으면 이름 맞춰 연결.
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* KillCountText;

private:
		// Quest_details 조합용 캐시 (남은시간/남은적이 서로 다른 이벤트로 오기 때문에 저장해둠)
	float CachedTimeRemaining = 0.f;
	int32 CachedEnemiesRemaining = 0;
	
	
	
	
	// 전투 피드백 위젯 부분.
public:
	// 적 처치 확정(킬로그) : 킬 피드에 한 줄 추가. 소유자: GameState(중계)
	UFUNCTION() void HandleEnemyKilled(EEnemyType EnemyType);
	
	//테스트용(일단 다른 코드 완성 후 테스트할 것).
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Test")
	void Test_SimulateCombatFeedback();
	
	
protected:
	// 히트마커: 크로스헤어 위에 겹칠 X자 이미지. 평소 숨김, 명중 순간만 표시
	UPROPERTY(meta = (BindWidgetOptional))
	UImage* HitMarkerImage;
 
	// 킬 피드가 쌓이는 세로 목록
	UPROPERTY(meta = (BindWidgetOptional))
	UVerticalBox* KillFeedBox;
 
	UPROPERTY(EditAnywhere, Category = "Feedback")
	TSubclassOf<UCH3DamageNumberWidget> DamageNumberWidgetClass;
 
	UPROPERTY(EditAnywhere, Category = "Feedback")
	TSubclassOf<UCH3KillFeedEntryWidget> KillFeedEntryWidgetClass;
 
	UPROPERTY(EditAnywhere, Category = "Feedback")
	float HitMarkerDuration = 0.12f;
 
	UPROPERTY(EditAnywhere, Category = "Feedback")
	FLinearColor HitMarkerNormalColor = FLinearColor::White;
 
	UPROPERTY(EditAnywhere, Category = "Feedback")
	FLinearColor HitMarkerCriticalColor = FLinearColor::Red;

	
	
private:
	
 
		// 히트마커 자동 숨김 타이머
		// n초 뒤에 히트 마커를 숨겨라.
		// 연속 명중 시, 이젠 예약을 취소하고 새로 걸어야해서.
	FTimerHandle HitMarkerTimerHandle;
 
		//타이머 시간이 다 된 후 호출.
	UFUNCTION()
	void HideHitMarker();
 
	
	
	//----------크로스헤어 부분
protected:
		// 십자를 한 장 이미지가 아니라 선분 4개로 나눔.
		// 이유: 뛰거나 할 때 벌어지게 하기 위해선 독립된 선분 4개 필요.
	UPROPERTY(meta = (BindWidgetOptional)) UImage* CrosshairTop;
	UPROPERTY(meta = (BindWidgetOptional)) UImage* CrosshairBottom;
	UPROPERTY(meta = (BindWidgetOptional)) UImage* CrosshairLeft;
	UPROPERTY(meta = (BindWidgetOptional)) UImage* CrosshairRight;

		// 십자가 중심에서 밀려나는 현재 거리(픽셀). 매 프레임 목표값을 향해 보간됨.
	float CurrentSpread = 8.f;

		// 상태별 목표 벌어짐(픽셀). 에디터에서 조정 가능하도록 노출.
		//Idle 상태
	UPROPERTY(EditAnywhere, Category = "Crosshair") float SpreadIdle = 8.f;    // 평상시
	UPROPERTY(EditAnywhere, Category = "Crosshair") float SpreadWalking = 16.f; // 걷기(조금 벌어짐)
	UPROPERTY(EditAnywhere, Category = "Crosshair") float SpreadRunning = 24.f; // 달릴 때
	UPROPERTY(EditAnywhere, Category = "Crosshair") float SpreadADS = 3.f;      // 정조준(나중에 정조준 코드 붙으면 사용)

		// 걷기/달리기를 가르는 두 임계값(cm/s)

	/*달리는 걸 속도로 판정했을 때. 지금은 sprint 키가 구분점이니까 필요 없음.
		// 달림 판정 기준 속도(cm/s). 이 값보다 빠르면 "달리는 중"으로 간주. => 나중에 달리기가 몇인지 확인하고 수정.
	UPROPERTY(EditAnywhere, Category = "Crosshair") float RunSpeedThreshold = 300.f;
	*/
	
	
		// [변경] RunSpeedThreshold(300) → WalkSpeedThreshold(50).
		// 역할이 바뀜: "달리는 중인가?"를 판정하던 값이 아니라,
		// "제자리인가, 움직이는 중인가?"만 가르는 값. 달림/걷기 구분은 bIsSprinting이 담당.
		// 50인 이유: 발판 미끄러짐 같은 미세 속도를 Idle로 처리하기 위한 여유값.
	UPROPERTY(EditDefaultsOnly, Category = "UI|Crosshair")
	float WalkSpeedThreshold = 50.f;
	
	

		// 벌어짐/조임 부드러움. 클수록 빠르게 반응.
	UPROPERTY(EditAnywhere, Category = "Crosshair") float SpreadInterpSpeed = 12.f;

		// 매 프레임 크로스헤어 벌어짐 갱신
	void UpdateCrosshair(float DeltaTime);
	
	
		// 획득한 강화 카드 목록 출력하는 인벤토리.
public:
	// 강화 확정 시 호출. 종류별 개수를 세고 목록 위젯을 갱신.
	//   OnUpgradeConfirmed 델리게이트 규격과 일치해야 함(EUpgradeType, AController*).
	//   (FOnUpgradeConfirmed 선언: CH3TeamProjectGameMode.h)
	UFUNCTION()
	void HandleUpgradeAcquired(EUpgradeType ChosenUpgrade, AController* ForPlayer);
	
	

protected:
	// 획득한 강화 종류별 개수. 카드 확정마다 +1.
	//   Heal은 일회성 회복이라 목록에서 제외(넣지 않음).
	TMap<EUpgradeType, int32> AcquiredUpgrades;

	// 강화 목록 위젯. WBP에서 클래스 지정.
	UPROPERTY(EditDefaultsOnly, Category = "CH3|Upgrade")
	TSubclassOf<class UCH3UpgradeListWidget> UpgradeListWidgetClass;

	// 화면에 붙일 목록 위젯 인스턴스. WBP의 UpgradeListSlot(NamedSlot 등)에 넣거나 뷰포트에 추가.
	UPROPERTY(meta = (BindWidgetOptional))
	UCH3UpgradeListWidget* UpgradeListWidget;
	
	
	// 게임 클리어/오버 결과 화면. WBP에서 클래스 지정.
	UPROPERTY(EditDefaultsOnly, Category = "CH3|Result")
	TSubclassOf<class UCH3ResultWidget> ResultWidgetClass;

	// 화면에 붙일 결과 위젯 인스턴스.
	UPROPERTY()
	class UCH3ResultWidget* ResultWidget;
	
		// 토글용.
public:
	void ToggleUpgradeInventory();
	
	/* -------------캐릭터 연결 없이 테스트할 시
private:
	// 스태미나: 목표값과 표시값을 분리 → NativeTick에서 서서히 따라가게 함
	float TargetStaminaPercent = 1.f;
	float DisplayedStaminaPercent = 1.f;

	// Quest_details 조합용 캐시 (남은시간/남은적 이벤트가 따로 와서 저장해둠)
	float CachedTimeRemaining = 0.f;
	int32 CachedEnemiesRemaining = 0;

	// 테스트용
	float TestHealth = 100.f;
	float TestStamina = 100.f;
	
	
	// 테스트용 탄약.
	int32 TestAmmo = 30;
	*/
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