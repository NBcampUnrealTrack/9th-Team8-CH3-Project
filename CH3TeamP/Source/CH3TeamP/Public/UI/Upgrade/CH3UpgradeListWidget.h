// CH3UpgradeListWidget.h
// 획득한 강화를 아이콘 + 수량(×N)으로 나열하는 목록.
// 데이터는 MainHUD가 소유하고, 이 위젯은 받아서 그리기만 함.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CH3GameplayTypes.h"		// EUpgradeType
#include "CH3UpgradeListWidget.generated.h"

class UWrapBox;
class UTexture2D;
class UCH3UpgradeSlotWidget;		// 인벤 내 슬롯 위젯

UCLASS()
class CH3TEAMP_API UCH3UpgradeListWidget : public UUserWidget
{
	GENERATED_BODY()

public:
		// (MainHUD에서 위젯으로) 종류별 개수 맵을 받아 목록을 다시 그림.
	void RefreshList(const TMap<EUpgradeType, int32>& Upgrades);

protected:
		// 아이콘들이 쌓이는 컨테이너. 항목이 늘면 자동 줄바꿈.
	UPROPERTY(meta = (BindWidget))
	UWrapBox* UpgradeWrapBox;

		// 강화 종류 -> 아이콘 텍스처. 카드와 같은 매핑을 WBP 클래스 디폴트에서 지정.
		//   (카드 텍스처를 그대로 재사용하면 됨)
	UPROPERTY(EditDefaultsOnly, Category = "CH3|Upgrade")
	TMap<EUpgradeType, UTexture2D*> UpgradeIcons;
	
		// 창이 열려 있는 동안 마우스 클릭이 게임으로 새어나가지 않게 막기.
		// 이걸 안 놓으니까 인벤 내 아이템 건들 때마다 타타타타탕!
		// (FInputModeGameAndUI는 UI가 안 잡은 클릭을 게임으로 흘려보내기 때문)
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	
	
	
	
	//	강화 카드를 보관하는 "인벤토리"
public:
	// 창 여닫기 토글. ACH3HUD가 I키 입력 시 호출.
	//   이 위젯 자체가 창 역할까지 겸함(격자 + 여닫기).
	void ToggleInventory(const TMap<EUpgradeType, int32>& CurrentUpgrades);

protected:
	// 닫기 버튼. I키로도 닫을 수 있어서 필수는 아님. 선택. 근데 난 넣을 거.
	UPROPERTY(meta = (BindWidgetOptional))
	class UButton* CloseButton;

	virtual void NativeOnInitialized() override;

private:
	UFUNCTION() void OnCloseClicked();

		// WBP에 배치된 슬롯들을 수집해 담아둔다. SlotIndex 순으로 정렬.
		// 강화 영역과 아이템 영역을 분리 보관 — 서로 섞이면 안 되므로.
	UPROPERTY()
	TArray<UCH3UpgradeSlotWidget*> UpgradeSlots;

	UPROPERTY()
	TArray<UCH3UpgradeSlotWidget*> ItemSlots;

		// 표시 순서. 드래그로 바뀌는 건 이 배열뿐 —
		// 실제 강화 효과(AcquiredUpgrades)는 절대 안 건드린다.
	UPROPERTY()
	TArray<EUpgradeType> UpgradeOrder;

		// WBP 트리를 훑어 슬롯을 찾아 배열에 담는다. NativeOnInitialized에서 1회만 호출.
	void CollectSlots();
	
};