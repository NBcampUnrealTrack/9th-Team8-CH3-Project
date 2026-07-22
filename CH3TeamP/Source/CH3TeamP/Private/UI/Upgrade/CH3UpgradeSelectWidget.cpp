// CH3UpgradeSelectWidget.cpp

#include "UI/Upgrade/CH3UpgradeSelectWidget.h"
#include "Components/Button.h"				// UButton. 클릭 이벤트(OnClicked) 바인딩용.
#include "Components/Image.h"				// UImage. SetBrushFromTexture용.
#include "Engine/Texture2D.h"				// UTexture2D.
#include "CH3TeamProjectGameMode.h"			// 선택 확정 전달(ConfirmUpgradeSelection) 호출용.


	//
void UCH3UpgradeSelectWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

		// BindWidget이 있어도 혹시 모를 null 대비. (포인터는 쓰기 전에 확인)
		// AddDynamic : "이 일이 일어나면 내 함수를 불러줘"라고 신청서 내는 거.
		// Card버튼이 OnClicked(클릭되면), 내(this) OnCardClicked함수를 불러줘(AddDynamic). 라고 해석.
	if (CardButton_0)
	{
		CardButton_0->OnClicked.AddDynamic(this, &UCH3UpgradeSelectWidget::OnCard0Clicked);
	}
	if (CardButton_1)
	{
		CardButton_1->OnClicked.AddDynamic(this, &UCH3UpgradeSelectWidget::OnCard1Clicked);
	}
	if (CardButton_2)
	{
		CardButton_2->OnClicked.AddDynamic(this, &UCH3UpgradeSelectWidget::OnCard2Clicked);
	}
}

void UCH3UpgradeSelectWidget::SetupCards(const TArray<EUpgradeType>& InCards)
{
	PresentedCards = InCards;

		// 3칸 고정 UI. (GameMode의 UpgradeCardCount 기본값 3에 맞춤)
		// 숫자 늘리고 싶으면 GameMode에서 변경할 것.
	UButton* Buttons[3] = { CardButton_0, CardButton_1, CardButton_2 };
	UImage* Images[3] = { CardImage_0, CardImage_1, CardImage_2 };

	if (PresentedCards.Num() > 3)
	{
			// UpgradeCardCount를 4 이상으로 올리면 UI도 칸을 늘려야 함. 그전까진 경고만.
		UE_LOG(LogTemp, Warning, TEXT("UpgradeSelectWidget: 카드 %d장이 제시됐지만 UI는 3장까지만 표시합니다."), PresentedCards.Num());
	}

	for (int32 i = 0; i < 3; ++i)
	{
		if (!Buttons[i] || !Images[i])
		{
			continue;
		}

		const bool bHasCard = PresentedCards.IsValidIndex(i);

			// 카드가 3장 미만이면 남는 칸은 Collapsed(자리도 차지 안 함)로 접는다.
		Buttons[i]->SetVisibility(bHasCard ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		if (!bHasCard)
		{
			continue;
		}

			// TMap::Find는 "값을 가리키는 포인터"를 돌려줌(키가 없으면 nullptr).
			// 우리 값 타입이 UTexture2D*라서 결과는 UTexture2D**(포인터의 포인터)가 된다.
		UTexture2D** FoundTexture = CardTextures.Find(PresentedCards[i]);
		if (FoundTexture && *FoundTexture)
		{
				// bMatchSize=false: 디자이너에서 정한 칸 크기를 유지하고 그림만 갈아끼움.
			Images[i]->SetBrushFromTexture(*FoundTexture, false);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("UpgradeSelectWidget: CardTextures 맵에 %d번 타입의 이미지가 없습니다! WBP 클래스 디폴트를 확인하세요."), (int32)PresentedCards[i]);
		}
	}
}

void UCH3UpgradeSelectWidget::OnCard0Clicked()
{
	HandleCardClicked(0);
}

void UCH3UpgradeSelectWidget::OnCard1Clicked()
{
	HandleCardClicked(1);
}

void UCH3UpgradeSelectWidget::OnCard2Clicked()
{
	HandleCardClicked(2);
}

void UCH3UpgradeSelectWidget::HandleCardClicked(int32 CardIndex)
{
	if (!PresentedCards.IsValidIndex(CardIndex))
	{
		return;
	}

	ACH3TeamProjectGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<ACH3TeamProjectGameMode>() : nullptr;
	if (!GM)
	{
		UE_LOG(LogTemp, Error, TEXT("UpgradeSelectWidget: GameMode를 찾지 못해 강화 선택을 전달할 수 없습니다."));
		return;
	}

		// 선택 "요청"만 보낸다. 여기서 위젯을 직접 닫지 않는 이유:
		// ConfirmUpgradeSelection이 거부될 수도 있는데(상태 불일치 등), 그때 카드만 사라지면
		// 일시정지는 유지된 채 조작 불가 = 소프트락. 닫기는 "확정 성공" 방송(OnUpgradeConfirmed)을
		// 받은 HUD가 처리한다. (요청과 확정을 분리)
	GM->ConfirmUpgradeSelection(PresentedCards[CardIndex]);
}