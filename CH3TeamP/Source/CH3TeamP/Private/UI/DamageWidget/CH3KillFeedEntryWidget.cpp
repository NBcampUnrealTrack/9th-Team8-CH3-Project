// [신규 파일] CH3KillFeedEntryWidget.cpp

#include "UI/DamageWidget/CH3KillFeedEntryWidget.h"
#include "Components/TextBlock.h"
#include "TimerManager.h"

void UCH3KillFeedEntryWidget::InitEntry(const FText& Message)
{
	if (KillText)
	{
		KillText->SetText(Message);
	}
}

void UCH3KillFeedEntryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 생성되는 순간 "LifeTime초 뒤에 RemoveSelf를 실행해줘" 라고 딱 한 번 예약.
	// 매 프레임 검사(Tick) 없이 정확한 시점에 한 번만 실행됨 → 비용 최소.
	GetWorld()->GetTimerManager().SetTimer(
		RemoveTimerHandle, this, &UCH3KillFeedEntryWidget::RemoveSelf, LifeTime, false);
}

void UCH3KillFeedEntryWidget::RemoveSelf()
{
	// VerticalBox에서 자신을 빼면 아래 줄들이 자동으로 위로 당겨짐
	RemoveFromParent();
}