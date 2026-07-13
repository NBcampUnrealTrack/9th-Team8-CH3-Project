// [신규 파일] CH3DamageNumberWidget.h
// [클래스 종류: UserWidget] Add C++ Class 마법사에서 "UserWidget" 검색해서 생성 후 내용 교체
//
// 역할: 적을 때렸을 때 그 위치에 뜨는 데미지 숫자 하나.
//       생성되면 스스로 위로 떠오르며 투명해지다가, 수명이 다하면 스스로 사라진다.
//
// [객체지향 포인트 - 자기 관리(Self-Managing) 객체]
//   이 위젯은 자기 수명/움직임/소멸을 "스스로" 처리한다.
//   MainHUD는 CreateWidget으로 낳기만 하고, 그 뒤는 신경 쓸 필요가 없다.
//   → MainHUD 코드가 단순해지고, 데미지 숫자가 동시에 10개 떠도 각자 알아서 굴러감.
//
// [왜 별도 클래스인가]
//   여러 적을 연달아 때리면 숫자가 "동시에 여러 개" 떠야 한다.
//   MainHUD에 텍스트 하나를 박아두는 방식으론 개수가 가변인 상황을 처리할 수 없음.
//   → 개수가 실행 중에 변하는 것은 반드시 독립 클래스 + CreateWidget 동적 생성.
//
// [선택하지 않은 방식: UWidgetComponent(3D 월드 부착)]
//   적 액터에 위젯 컴포넌트를 붙여 머리 위에 띄우는 방법도 있으나,
//   - 적 클래스(타 팀원 담당)를 수정해야 하고
//   - 적이 죽어 사라지면 숫자도 같이 사라지는 문제가 있어
//   화면(뷰포트)에 직접 띄우고 월드좌표→화면좌표로 투영하는 방식을 선택함.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CH3DamageNumberWidget.generated.h"

class UTextBlock;

UCLASS()
class CH3TEAMP_API UCH3DamageNumberWidget : public UUserWidget
{
	GENERATED_BODY()

public:
		// 생성 직후 MainHUD가 딱 한 번 호출. 표시할 데미지와 크리티컬 여부 받기.
	void InitDamageNumber(float DamageAmount, bool bIsCritical);

protected:
	// 매 프레임 Tick : 위로 이동 + 서서히 투명화 + 수명 끝나면 자멸
	virtual void NativeTick(const FGeometry& MyGeometry, float DeltaTime) override;

		// 숫자를 표시할 텍스트 (WBP에서 같은 이름의 Text 위젯 배치)
	UPROPERTY(meta = (BindWidget))
	UTextBlock* DamageText;

		// 화면에 떠있는 총 시간(초)
	UPROPERTY(EditAnywhere, Category = "DamageNumber")
	float LifeTime = 0.8f;

		// 초당 떠오르는 속도(픽셀)
	UPROPERTY(EditAnywhere, Category = "DamageNumber")
	float RiseSpeed = 60.f;

		// 일반 타격 글자 색
	UPROPERTY(EditAnywhere, Category = "DamageNumber")
	FLinearColor NormalColor = FColor::White;
	
		// 크리티컬일 때 글자 색
	UPROPERTY(EditAnywhere, Category = "DamageNumber")
	FLinearColor CriticalColor = FLinearColor(FColor(149, 0, 0)); // 레드
	
		// 크리티컬일 때 사용할 폰트 (비워두면 기존 폰트 유지)
	UPROPERTY(EditAnywhere, Category = "DamageNumber")
	TObjectPtr<UObject> CriticalFontObject;

		// 크리 테두리 두께
	UPROPERTY(EditAnywhere, Category = "DamageNumber")
	int32 CriticalOutlineSize = 2;

		// 크리 테두리 색
	UPROPERTY(EditAnywhere, Category = "DamageNumber")
	FLinearColor CriticalOutlineColor = FLinearColor::White;

private:
	float ElapsedTime = 0.f; // 생성 후 흐른 시간
};