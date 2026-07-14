// CH3HUD.h
	// 이 플레이어의 화면에 MainHUD 위젯을 "언제 만들지" 결정하고 실제로 만드는 관리자 역할.
	// 커스텀 PlayetController를 안 만들꺼임. 그거 대체용.
	// 아ㅋㅋ 어차피 Owning Player만 대체할 수 있으면 된다고 ㅋㅋ.
	// 엔진 내에서 원래 있던 PlayerController가 게임 시작 시 자동으로 하나씩 만들어서 데리고 다님.
	// UI 소유권을 한 액터에 모으니까 오히려 이득인가? 객체지향?
	// 얘 작성하고, 엔진 에디터의 GameMode에 얘 등록해야함.

	// 역할 1 : 메인 HUD.
	// 역할 2 : Pause 화면.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "CH3HUD.generated.h"

class UCH3MainHUDWidget;
class UCH3PauseWidget;

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
	
	
	
private:
	// 실제 열기 동작.
	void OpenPauseMenu();

	// 지금 일시정지 메뉴를 열어도 되는 상태인지 GameState에 물어보는 게이트.(두유 워너 오픈 퍼즈)
	bool CanOpenPauseMenu() const;
	
};