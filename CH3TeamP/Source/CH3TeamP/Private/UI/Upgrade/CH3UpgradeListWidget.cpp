// CH3UpgradeListWidget.cpp

#include "UI/Upgrade/CH3UpgradeListWidget.h"
#include "Components/WrapBox.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/HorizontalBox.h"
#include "Components/Button.h"
#include "Blueprint/WidgetTree.h"				// WidgetTree-> GetAllWidgets 사용
#include "UI/Upgrade/CH3UpgradeSlotWidget.h"	// 슬롯 위젯


	// 카드 인벤토리 열고 닫을 때, NativeOnInitialized 필요해서, 강화 리스트 위에 배치.
void UCH3UpgradeListWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (CloseButton)
	{
		CloseButton->OnClicked.AddDynamic(this, &UCH3UpgradeListWidget::OnCloseClicked);
	}
	
		// WBP에 배치된 슬롯들을 여기서 1회만 수집.
		//  NativeConstruct가 아닌 이유: 창을 열 때마다 다시 수집할 이유가 없음.
	CollectSlots();
}

// I키로 여닫기. 이 위젯이 창까지 겸함.
void UCH3UpgradeListWidget::ToggleInventory(const TMap<EUpgradeType, int32>& CurrentUpgrades)
{
	if (IsInViewport())
	{
		// 열려 있으면 닫기.
		RemoveFromParent();
	}
	else
	{
		// 닫혀 있으면 열면서 최신 목록으로 갱신.
		AddToViewport(40);		// 카드(60)보단 아래, HUD보단 위
		RefreshList(CurrentUpgrades);
	}
}

void UCH3UpgradeListWidget::OnCloseClicked()
{
	RemoveFromParent();
}


// 카드 강화 리스트 내용.
void UCH3UpgradeListWidget::RefreshList(const TMap<EUpgradeType, int32>& Upgrades)
{
	// 1) 순서 배열 갱신
	//  새로 획득한 강화만 뒤에 추가한다. 이미 있던 건 자리를 그대로 둠 —
	//  드래그로 바꾼 순서가 카드를 먹을 때마다 흐트러지면 안 되기 때문.
	for (const TPair<EUpgradeType, int32>& Pair : Upgrades)
	{
		if (Pair.Value <= 0)
		{
			continue;
		}

		if (!UpgradeOrder.Contains(Pair.Key))
		{
			UpgradeOrder.Add(Pair.Key);
		}
	}

	// 2) 순서대로 슬롯에 내용 전달
	for (int32 i = 0; i < UpgradeSlots.Num(); ++i)
	{
		UCH3UpgradeSlotWidget* TargetSlot = UpgradeSlots[i];
		if (!TargetSlot)
		{
			continue;
		}

		// 순서 배열보다 슬롯이 많으면 나머지는 빈 칸으로.
		if (!UpgradeOrder.IsValidIndex(i))
		{
			TargetSlot->ClearSlot();
			continue;
		}

		const EUpgradeType Type = UpgradeOrder[i];

		// 개수는 맵에서 조회. 없으면 0 → 슬롯이 알아서 빈 칸 처리함.
		const int32* FoundCount = Upgrades.Find(Type);
		const int32 Count = FoundCount ? *FoundCount : 0;

		// 아이콘은 이 위젯이 들고 있다가 넘겨준다. 슬롯은 그림을 직접 갖지 않음.
		UTexture2D** FoundTex = UpgradeIcons.Find(Type);
		UTexture2D* Texture = FoundTex ? *FoundTex : nullptr;

		TargetSlot->UpdateSlot(Type, Count, Texture);
	}
}




	//인벤토리 내 슬롯.
void UCH3UpgradeListWidget::CollectSlots()
{
	UpgradeSlots.Empty();
	ItemSlots.Empty();

	if (!WidgetTree)
	{
		return;
	}

		// WBP 안의 모든 위젯을 훑어 슬롯만 골라냄.
		// 이렇게 하면 나중에 WBP에 칸을 추가해도 코드 수정 없이 자동으로 잡힘.
	TArray<UWidget*> AllWidgets;
	WidgetTree->GetAllWidgets(AllWidgets);

	for (UWidget* Widget : AllWidgets)
	{
		UCH3UpgradeSlotWidget* FoundSlot = Cast<UCH3UpgradeSlotWidget>(Widget);
		if (!FoundSlot)
		{
			continue;
		}

			// 슬롯이 스스로 밝힌 소속에 따라 나눠 담음. (강화 영역 / 아이템 영역)
		if (FoundSlot->SlotType == EUpgradeSlotType::Item)
		{
			ItemSlots.Add(FoundSlot);
		}
		else
		{
			UpgradeSlots.Add(FoundSlot);
		}
	}

		// GetAllWidgets의 반환 순서는 보장되지 않으므로, SlotIndex 기준으로 직접 정렬.
		// 이걸 해야 배열 순서와 화면 배치 순서가 어긋나지 않음.
	UpgradeSlots.Sort([](const UCH3UpgradeSlotWidget& A, const UCH3UpgradeSlotWidget& B)
	{
		return A.SlotIndex < B.SlotIndex;
	});

	ItemSlots.Sort([](const UCH3UpgradeSlotWidget& A, const UCH3UpgradeSlotWidget& B)
	{
		return A.SlotIndex < B.SlotIndex;
	});

		// 디버그용. 슬롯이 제대로 잡혔는지 확인용. 확인 후 삭제.
	UE_LOG(LogTemp, Warning, TEXT("[Inventory] 슬롯 수집: 강화 %d개, 아이템 %d개"),
		UpgradeSlots.Num(), ItemSlots.Num());
}

