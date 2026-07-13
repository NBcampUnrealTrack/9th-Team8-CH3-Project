// CH3HUD.h
// 역할: 이 플레이어의 화면에 MainHUD 위젯을 "언제 만들지" 결정하고 실제로 만드는 관리자 역할.
		//커스텀 PlayetController가 없어서 대체하는 수단 중 하나.
		//PlayerController가 게임 시작 시 자동으로 하나씩 만들어서 데리고 다님.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "CH3HUD.generated.h"

class UCH3MainHUDWidget;

UCLASS()
class CH3TEAMP_API ACH3HUD : public AHUD
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

		// 에디터에서 WBP_MainHUD(만들어둔 위젯 블루프린트)를 여기에 연결.
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UCH3MainHUDWidget> MainHUDClass;

		// 실제로 생성된 위젯 인스턴스를 담아둠 (중복 생성 방지 등에 나중에 활용 가능)
	UPROPERTY()
	UCH3MainHUDWidget* MainHUD;
};