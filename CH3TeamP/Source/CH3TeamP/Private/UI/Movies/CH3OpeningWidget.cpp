// CH3OpeningWidget.cpp
#include "UI/Movies/CH3OpeningWidget.h"
#include "MediaPlayer.h"
#include "MediaSource.h"
#include "Kismet/GameplayStatics.h"

void UCH3OpeningWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
		// 키 입력을 받으려면 이 위젯이 포커스 가능해야 함.
	SetIsFocusable(true);

	if (MediaPlayer)
	{
		// 블루프린트에서 재현 안 되던 델리게이트 바인딩을 C++에서 직접 건다.
		MediaPlayer->OnEndReached.AddDynamic(this, &UCH3OpeningWidget::HandlePlaybackEnded);

		if (MediaSource)
		{
			MediaPlayer->OpenSource(MediaSource);
		}
	}
	
	
		// 키 입력(스킵)을 받으려면 이 위젯이 포커스를 가져야 함(건너뛰기 용도).
	SetKeyboardFocus();
}

void UCH3OpeningWidget::NativeDestruct()
{
	if (MediaPlayer)
	{
		MediaPlayer->OnEndReached.RemoveDynamic(this, &UCH3OpeningWidget::HandlePlaybackEnded);
	}
	Super::NativeDestruct();
}

void UCH3OpeningWidget::HandlePlaybackEnded()
{
	UGameplayStatics::OpenLevel(this, NextLevelName);
}


FReply UCH3OpeningWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	// 스페이스바나 Enter로 스킵. 재생 끝나기를 기다리지 않고 바로 레벨 이동.
	if (InKeyEvent.GetKey() == EKeys::SpaceBar || InKeyEvent.GetKey() == EKeys::Enter)
	{
		HandlePlaybackEnded();
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}