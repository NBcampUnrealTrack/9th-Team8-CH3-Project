// [신규 파일] CH3KillFeedEntryWidget.h
// [클래스 종류: UserWidget] Add C++ Class 마법사에서 "UserWidget" 검색해서 생성 후 내용 교체
//
// 역할: "일반 좀비 처치!" 같은 킬 확정 메시지 한 줄.
//       MainHUD의 세로 목록(KillFeedBox)에 추가되고, 몇 초 뒤 스스로 사라진다.
//
// [데미지 숫자와의 설계 차이 - 같은 자멸인데 왜 방법이 다른가]
//   DamageNumber: 매 프레임 "움직여야" 해서 NativeTick에서 수명까지 같이 처리 (어차피 Tick이 돌므로)
//   KillFeed:     생겼다가 사라지기만 하면 됨 → Tick을 아예 안 돌리고 타이머 1개만 예약
//   → "필요한 만큼만 비용을 쓴다"는 원칙. 안 움직이는 위젯에 Tick을 돌리는 건 낭비.
//
// [선택하지 않은 방식: ListView]
//   UMG의 ListView는 수백 개 항목을 스크롤할 때 위젯을 재활용해주는 고급 컨테이너.
//   킬 피드는 동시에 3~5줄 수준이라 오버스펙이고, EntryWidget 인터페이스 구현 등
//   진입 장벽만 높아짐 → VerticalBox + 수동 생성/자멸이 이 규모의 표준적 선택.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CH3KillFeedEntryWidget.generated.h"

class UTextBlock;

UCLASS()
class CH3TEAMP_API UCH3KillFeedEntryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
		// 생성 직후 MainHUD가 호출. 표시할 문장을 받는다 (예: "돌격형 처치!")
	void InitEntry(const FText& Message);

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* KillText;

		// 목록에 떠있는 시간(초). 지나면 스스로 사라짐
	UPROPERTY(EditAnywhere, Category = "KillFeed")
	float LifeTime = 3.f;

private:
	FTimerHandle RemoveTimerHandle;

	UFUNCTION()
	void RemoveSelf();
};