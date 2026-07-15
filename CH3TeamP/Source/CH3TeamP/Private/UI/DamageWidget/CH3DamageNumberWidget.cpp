// [신규 파일] CH3DamageNumberWidget.cpp

#include "UI/DamageWidget/CH3DamageNumberWidget.h"
#include "Components/TextBlock.h"

void UCH3DamageNumberWidget::InitDamageNumber(float DamageAmount, bool bIsCritical)
{
	if (DamageText)
	{
			// 대미지를 혹시 소수점이 발생할 시, 소수점 없는 정수로 표시 (37.5 → "38")
		DamageText->SetText(FText::AsNumber(FMath::RoundToInt(DamageAmount)));
		DamageText->SetColorAndOpacity(bIsCritical ? CriticalColor : NormalColor);
	}

	if (bIsCritical)
	{
			// 크리티컬 대미지량 표시는 살짝 크게 (1.3배). RenderScale은 레이아웃 계산 없이 겉모습만 키움.
		SetRenderScale(FVector2D(1.3f, 1.3f));
	}
}

void UCH3DamageNumberWidget::NativeTick(const FGeometry& MyGeometry, float DeltaTime)
{
	Super::NativeTick(MyGeometry, DeltaTime);

	ElapsedTime += DeltaTime;

		// 1) 위로 떠오르기: 생성 위치 기준으로 매 프레임 조금씩 위로(-Y) 이동
	SetRenderTranslation(FVector2D(0.f, -RiseSpeed * ElapsedTime));

		// 2) 서서히 투명해지기: 수명 대비 진행률로 불투명도를 1→0으로
		//    (0.5초 지점부터 사라지기 시작하게 하면 "잠깐 보였다 스르륵" 느낌)
	const float FadeStart = LifeTime * 0.5f;
	if (ElapsedTime > FadeStart)
	{
		const float FadeAlpha = 1.f - (ElapsedTime - FadeStart) / (LifeTime - FadeStart);
		SetRenderOpacity(FMath::Clamp(FadeAlpha, 0.f, 1.f));
	}

		// 3) 수명 종료 → 스스로 화면에서 제거 (자기 관리의 핵심)
		//    RemoveFromParent 후엔 참조가 없으면 가비지 컬렉터가 알아서 메모리 회수.
	if (ElapsedTime >= LifeTime)
	{
		RemoveFromParent();
	}
}