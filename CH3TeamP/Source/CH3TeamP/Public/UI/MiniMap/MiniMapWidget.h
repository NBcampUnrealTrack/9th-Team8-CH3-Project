// [수정 파일] MiniMapWidget.h
// [클래스 종류: UserWidget] (기존 파일 교체 — 새로 만들 필요 없음)
//
// ★ 버그 수정: 클래스 이름이 파일마다 3가지로 달랐음 ★
//   - 이 파일:              UMiniMapWidget
//   - MainHUD .h 전방선언:  UCH3MinimapMarkerWidget
//   - MainHUD .h 멤버 타입:  UMiniMapMarkerWidget   ← 오타로 또 다른 이름
//   - MainHUD .cpp:         UCH3MinimapMarkerWidget + 존재하지 않는 SetMarkerAngle() 호출
//   → "UCH3MinimapMarkerWidget" 하나로 통일하고, 호출되던 함수들을 실제로 구현.
//
// [클래스명 변경 후 에디터에서 꼭 할 일]
//   기존 WBP(마커 블루프린트)가 옛 클래스(UMiniMapWidget)를 부모로 알고 있어 연결이 끊김.
//   → WBP 열기 → 상단 메뉴 File → Reparent Blueprint → UCH3MinimapMarkerWidget 선택
//   → 디자이너에서 Image 위젯 이름도 BlipIcon → MarkerIcon 으로 변경
//
// 역할: 미니맵 위에 찍히는 "점 하나" (플레이어/적/장애물 공용).
//       색과 회전만 바꿔서 여러 용도로 재사용한다. (재사용성 = 클래스를 나눈 이유)

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MiniMapWidget.generated.h"

class UImage;

UCLASS()
class CH3TEAMP_API UCH3MinimapMarkerWidget : public UUserWidget
{
	GENERATED_BODY()

public:
		// 마커 색 변경. 플레이어=흰색, 적=빨강 식으로 하나의 위젯을 여러 용도로 재사용
	void SetMarkerColor(const FLinearColor& NewColor);

		// 마커 회전(도 단위). 플레이어가 바라보는 방향을 화살표로 표현할 때 사용.
		// MainHUD의 NativeTick에서 이미 호출 중이던 함수 — 선언/구현이 없어서 컴파일 에러였음
	void SetMarkerAngle(float AngleDegrees);

protected:
	// Marker 사용
	UPROPERTY(meta = (BindWidget))
	UImage* MarkerIcon;
};