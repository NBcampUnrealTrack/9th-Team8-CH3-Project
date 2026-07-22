// CH3UpgradeSlotWidget.h
	// 강화 인벤토리의 칸 하나. 아이콘 + 수량을 표시.
	// 슬롯은 자기 "정체", 그니까 칸에 정해진 아이템을 지니는 게 아님. "위치(칸 번호)"만 알도록.
	//  내용물은 인벤토리 위젯이 실행 중에 넣어줌. -> 드래그로 순서 변경이 가능.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CH3GameplayTypes.h"		// EUpgradeType. generated.h보다 위여야 함.
#include "CH3UpgradeSlotWidget.generated.h"

class UImage;
class UTextBlock;

	// 인벤에 넣을 아이템 열거형.
	// 슬롯이 어느 영역에 속하는지 구분. UI 전용 개념이라 이 헤더에 둔다.
	// (게임 로직의 CH3GameplayTypes.h는 팀 공용이라 UI 사정으로 건드리지 않음)
UENUM(BlueprintType)
enum class EUpgradeSlotType : uint8
{
	Upgrade,		// 윗줄 — 획득한 강화 표시
	Item			// 아랫줄 — 사용 가능한 아이템(회복 등). 지금은 밑설계만.
};

UCLASS()
class CH3TEAMP_API UCH3UpgradeSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
		// (인벤토리에서 슬롯으로) 이 칸에 표시할 내용을 통째로 받아 갱신한다.
		// Texture가 null이거나 Count가 0이면 "빈 칸"으로 표시.
	void UpdateSlot(EUpgradeType InType, int32 Count, UTexture2D* Texture);

	// (인벤토리에서 슬롯으로) 이 칸을 비우기. 템 사용하거나, 미획득, 순서 변경 등.
	void ClearSlot();

		// 지금 이 칸에 들어있는 강화 종류. 드래그 시 "무엇을 옮기는지" 알기 위해 필요.
		// 빈 칸이면 bIsEmpty가 true라 이 값은 의미 없음.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CH3|Slot")
	EUpgradeType CurrentType = EUpgradeType::AttackUp;

		// 이 칸이 비어 있는지. 드래그 대상 판정에 쓰임.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CH3|Slot")
	bool bIsEmpty = true;

		// 이 칸의 번호. WBP 인스턴스마다 에디터에서 0,1,2... 지정.
		// 순서 배열의 몇 번째를 담당하는지를 뜻함.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CH3|Slot")
	int32 SlotIndex = 0;

		// 이 칸이 강화 영역인지 아이템 영역인지. WBP에서 지정.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CH3|Slot")
	EUpgradeSlotType SlotType = EUpgradeSlotType::Upgrade;

protected:
		// 아이콘 이미지. WBP에서 같은 이름의 Image를 배치해야 연결됨.
	UPROPERTY(meta = (BindWidget))
	UImage* IMG_Icon;

		// 수량 텍스트("x3"). WBP에서 같은 이름의 TextBlock 배치.
	UPROPERTY(meta = (BindWidget))
	UTextBlock* TXT_Count;
};