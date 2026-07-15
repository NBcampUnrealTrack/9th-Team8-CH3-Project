// [수정 파일] MiniMapWidget.cpp — 비어있던 구현부 채움

#include "UI/MiniMap/MiniMapWidget.h"
#include "Components/Image.h"

void UCH3MinimapMarkerWidget::SetMarkerColor(const FLinearColor& NewColor)
{
	if (MarkerIcon)
	{
		// SetColorAndOpacity: 이미지에 색을 "곱해서" 입힘.
		// 흰색 원본 텍스처를 쓰면 어떤 색이든 깨끗하게 물들일 수 있다.
		// (원본이 이미 빨간 이미지면 파란색을 곱해도 검게 나오니, 마커 원본은 흰색 권장)
		MarkerIcon->SetColorAndOpacity(NewColor);
	}
}

void UCH3MinimapMarkerWidget::SetMarkerAngle(float AngleDegrees)
{
	if (MarkerIcon)
	{
		// RenderTransformAngle: 위젯을 화면상에서 회전(레이아웃엔 영향 없음).
		// 플레이어 마커를 삼각형/화살표로 만들면 "내가 보는 방향"이 미니맵에 표시됨.
		MarkerIcon->SetRenderTransformAngle(AngleDegrees);
	}
}