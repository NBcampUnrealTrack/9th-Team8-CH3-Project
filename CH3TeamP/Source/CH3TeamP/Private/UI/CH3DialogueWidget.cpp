// CH3DialogueWidget.cpp

#include "UI/CH3DialogueWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "TimerManager.h"

void UCH3DialogueWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 평소(초기) 상태는 플레이어 초상화.
	if (PortraitImage && PlayerIconTexture)
	{
		PortraitImage->SetBrushFromTexture(PlayerIconTexture, false);
	}
}

void UCH3DialogueWidget::ShowDialogue(const FText& Message, bool bUseNPCIcon)
{
	if (DialogueText)
	{
		DialogueText->SetText(Message);
	}

	if (PortraitImage)
	{
		UTexture2D* Texture = bUseNPCIcon ? NPCIconTexture : PlayerIconTexture;
		if (Texture)
		{
			PortraitImage->SetBrushFromTexture(Texture, false);
		}
	}

	// 매번 새로 뜰 때마다 수명 타이머를 재설정.
	GetWorld()->GetTimerManager().SetTimer(
		RemoveTimerHandle, this, &UCH3DialogueWidget::RemoveSelf, LifeTime, false);
}

void UCH3DialogueWidget::RemoveSelf()
{
	// 화면에서만 숨김(제거 아님) — 이 위젯은 재사용되며 항상 화면에 상주.
	// 킬피드처럼 매번 새로 만들지 않고, 하나를 계속 재사용하는 구조라 RemoveFromParent 대신 숨김 처리.
	SetVisibility(ESlateVisibility::Collapsed);
}