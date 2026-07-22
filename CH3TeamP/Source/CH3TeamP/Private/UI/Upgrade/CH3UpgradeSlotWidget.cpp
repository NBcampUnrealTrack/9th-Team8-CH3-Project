// CH3UpgradeSlotWidget.cpp

#include "UI/Upgrade/CH3UpgradeSlotWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"



void UCH3UpgradeSlotWidget::UpdateSlot(EUpgradeType InType, int32 Count, UTexture2D* Texture)
{
		// 개수가 0 이하거나 그림이 없으면 표시할 게 없으므로 빈 칸 처리로 넘김.
		// (미획득 강화, 아직 안 쓰는 아이템 칸이 여기로 옴)
	if (Count <= 0 || !Texture)
	{
		ClearSlot();
		return;
	}

	
	
		// 이 칸이 지금 무엇을 담고 있는지 기록. 나중에 드래그할 때 "무엇을 옮기는지" 판단 근거.
	CurrentType = InType;
	bIsEmpty = false;

	if (IMG_Icon)
	{
		IMG_Icon->SetBrushFromTexture(Texture, false);

			// ClearSlot에서 투명하게 만들어둔 것을 반드시 원래대로 되돌린다.
			// 슬롯은 재사용되므로, 되돌리지 않으면 내용이 들어와도 안 보임.
		IMG_Icon->SetColorAndOpacity(FLinearColor::White);
	}

	if (TXT_Count)
	{
		TXT_Count->SetText(FText::FromString(FString::Printf(TEXT("x%d"), Count)));
	}
}



void UCH3UpgradeSlotWidget::ClearSlot()
{
		// 빈 칸 상태로 되돌림. 위젯을 지우지 않고 "비어 보이게"만 함 — 칸 자체는 남아야
		// 나중에 드래그로 여기에 무언가를 놓을 수 있기 때문.
	bIsEmpty = true;

	if (IMG_Icon)
	{
			// 아이콘을 투명하게 만들어 빈 칸으로 보이게 함.
			// SetBrushFromTexture(nullptr)은 기본 흰 사각형이 남을 수 있어 알파 0을 씀.
		IMG_Icon->SetColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, 0.f));
	}

	if (TXT_Count)
	{
		TXT_Count->SetText(FText::GetEmpty());
	}
}