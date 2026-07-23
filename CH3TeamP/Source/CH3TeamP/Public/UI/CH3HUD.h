// CH3HUD.h
	// 이 플레이어의 화면에 MainHUD 위젯을 "언제 만들지" 결정하고 실제로 만드는 관리자 역할.
	// 커스텀 PlayetController를 안 만들꺼임. 그거 대체용.
	// 아ㅋㅋ 어차피 Owning Player만 대체할 수 있으면 된다고 ㅋㅋ.
	// 엔진 내에서 원래 있던 PlayerController가 게임 시작 시 자동으로 하나씩 만들어서 데리고 다님.
	// UI 소유권을 한 액터에 모으니까 오히려 이득인가? 객체지향?
	// 얘 작성하고, 엔진 에디터의 GameMode에 얘 등록해야함.

	// 역할 1 : 메인 HUD.
	// 역할 2 : Pause 화면.
	// 역할 3 : 강화 카드 선택 화면.		// [카드 강화 추가 7/19] 역할 하나 추가.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "CH3GameplayTypes.h"		// [카드 강화 추가 7/19] EUpgradeType. 아래 강화 핸들러 시그니처에 필요. (generated.h보다 위여야 함)
#include "CH3HUD.generated.h"

class UCH3MainHUDWidget;
class UCH3PauseWidget;
class UCH3UpgradeSelectWidget;		// [카드 강화 추가 7/19] 강화 카드 선택 위젯.
class AController;					// [카드 강화 추가 7/19] HandleUpgradeConfirmed 파라미터 타입.

UCLASS()
class CH3TEAMP_API ACH3HUD : public AHUD
{
	GENERATED_BODY()

public:

		// ESC(테스트는 P)를 누를 때마다 호출. 열려 있으면 닫고, 닫혀 있으면 (게이트 통과 시) 열기.
	void TogglePauseMenu();

		// 메뉴 닫기 + 게임 재개. PauseWidget의 Resume 버튼도 이 함수를 호출.
		// SetGamePaused(false)를 부르는 곳을 이 함수 하나로 일원화하기 위해 public.
	void ClosePauseMenu();
	
		// I키로 강화 획득 인벤토리 창을 여닫는 진입점. MainHUD 위젯의 토글을 호출.
	void ToggleUpgradeInventory();
	
protected:
	virtual void BeginPlay() override;

		// 에디터에서 WBP_MainHUD(만들어둔 위젯 블루프린트)를 여기에 연결.
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UCH3MainHUDWidget> MainHUDClass;
	
		// 실제로 생성된 위젯 인스턴스를 담아두는 곳. (중복 생성 방지 등. 나중에 활용할듯.)
	UPROPERTY()
	UCH3MainHUDWidget* MainHUD;
	
	//-------일시정지 부분.
		// BP_CH3HUD에서 WBP_Pause를 지정. MainHUDClass와 완전히 동일한 패턴.
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UCH3PauseWidget> PauseWidgetClass;
	
		// 일시정지 위젯 인스턴스. 닫을 때 파괴하지 않고 화면에서만 내림. 그래서 재사용 가능.
	UPROPERTY()
	UCH3PauseWidget* PauseWidget;
	
	//-------강화 카드 부분.		// [카드 강화 추가 7/19] 여기부터 새 멤버.
		// BP_CH3HUD에서 WBP_UpgradeSelect를 지정. PauseWidgetClass와 완전히 동일한 패턴.
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UCH3UpgradeSelectWidget> UpgradeSelectWidgetClass;

		// 강화 카드 위젯 인스턴스. Pause처럼 닫을 때 파괴하지 않고 재사용.
	UPROPERTY()
	UCH3UpgradeSelectWidget* UpgradeSelectWidget;
	
private:
	// 실제 열기 동작.
	void OpenPauseMenu();

	// 지금 일시정지 메뉴를 열어도 되는 상태인지 GameState에 물어보는 게이트.(두유 워너 오픈 퍼즈)
	bool CanOpenPauseMenu() const;

		// [카드 강화 추가 7/19] ----- 강화 카드 핸들러들 -----
		// [게임모드 → HUD] 카드 제시 방송 수신 → 위젯 표시 + 입력모드 전환.
		// AddDynamic(동적 델리게이트)에 바인딩하는 함수는 반드시 UFUNCTION이어야 함.
	UFUNCTION()
	void HandleUpgradeCardsPresented(const TArray<EUpgradeType>& Cards);

		// [카드 강화 추가 7/19] [게임모드 → HUD] 선택 "확정" 방송 수신 → 위젯 내리기 + 입력모드 복구.
		// 확정 방송이 올 때만 닫는다. (위젯 쪽 주석 참고: 소프트락 방지)
	UFUNCTION()
	void HandleUpgradeConfirmed(EUpgradeType ChosenUpgrade, AController* ForPlayer);
	
	
	
	
	//-----------------디버그용.
	
		// [카드 강화 추가 7/19] *****PIE 테스트용 — 배포 전 삭제할 것.*****
		// 전투 파트가 트리거(레벨업/웨이브)를 연결하기 전까지, L키로 강제 카드 발동.
		// [레벨업 추가 수정 7/23] 경험치 획득용으로 수정함. - .h에서 수정한 건 없으나 메모용.
	void DebugTriggerLevelUp();
	
	
	
		// [히트마커 디버그 추가 7/19] -> PIE에서 테스트하는 용도(배포 전 삭제)
		// H키로 히트마커 + 데미지 숫자를 강제 발동. 전투 파트 델리게이트 연결 전 임시 수단.
	void DebugTriggerHit();
	
	
		// PIE 테스트용 — 배포 전 삭제할 것.
		// 체력을 10씩 깎아 저체력 연출을 확인하는 디버그 키.
	void DebugDamagePlayer();
	
};