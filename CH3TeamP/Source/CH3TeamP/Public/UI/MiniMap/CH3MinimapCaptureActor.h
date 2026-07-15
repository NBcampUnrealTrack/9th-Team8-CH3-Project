// [신규 파일] CH3MinimapCaptureActor.h
// [클래스 종류: Actor] Add C++ Class 마법사에서 "Actor" 선택 후,
//                      생성된 파일 내용을 이 코드로 교체하면 됩니다.
//
// 역할: 게임 시작 시 맵 전체를 위에서 "딱 한 번" 촬영해서 RenderTarget에 저장.
//       이후에는 다시 안 찍으므로 매 프레임 렌더링 비용이 없음.
//
// [왜 이 방식인가 - 선택지 비교]
//   1) PNG 스크린샷: 촬영 해상도가 고정 → 확대 시 화질 깨짐 (기존 문제)
//   2) 매 프레임 SceneCapture: 화질 좋지만 GPU가 씬을 매 프레임 2번 그림 → 낭비
//   3) 1회 캡쳐(이 방식): RenderTarget 해상도를 크게(2048~4096) 잡으면
//      확대해도 선명 + 비용은 게임 시작 시 1번뿐 → 고정 맵에 최적
//
// [객체지향 포인트 - Single Source of Truth(단일 진실 공급원)]
//   미니맵 좌표 계산에 필요한 "촬영 범위" 값을 이 액터가 소유하고,
//   HUD는 이 액터에게 물어봐서(Getter) 가져다 씀.
//   → 촬영 범위를 바꿀 때 이 액터의 값 하나만 고치면 HUD 계산이 자동으로 맞춰짐.
//   (예전 방식: HUD에 좌표를 하드코딩 → 카메라 옮길 때마다 두 군데를 고쳐야 했음)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CH3MinimapCaptureActor.generated.h"

class USceneCaptureComponent2D;

UCLASS()
class CH3TEAMP_API ACH3MinimapCaptureActor : public AActor
{
	GENERATED_BODY()

public:
	ACH3MinimapCaptureActor();
	
		// 매 프레임 플레이어 머리 위로 이동 (촬영 자체는 컴포넌트의 bCaptureEveryFrame이 담당)
	virtual void Tick(float DeltaSeconds) override;


		// HUD가 마커 좌표 계산에 쓸 "한 화면에 담기는 월드 폭/높이"를 돌려줌.
		// RenderTarget이 정사각형이므로 가로 = 세로 = OrthoWidth.
		// 이 값은 게임 중 변하지 않으므로 HUD가 시작 시 1회만 받아 캐시해도 안전.
	FVector2D GetMinimapWorldSize() const;

protected:
	virtual void BeginPlay() override;

		// 실제 촬영을 담당하는 카메라 컴포넌트
	UPROPERTY(VisibleAnywhere, Category = "Minimap")
	USceneCaptureComponent2D* CaptureComponent;

		// 미니맵에 담을 월드 폭(유닛). 8000 = 플레이어 기준 반경 40m.
		// 너무 크면 사물 구별이 안 되고, 너무 작으면 답답함 → 에디터에서 조정하며 결정.
	UPROPERTY(EditAnywhere, Category = "Minimap", meta = (ClampMin = "1"))
	float OrthoWidth = 4000.f;
 
		// 카메라의 절대 고도(월드 Z). 플레이어 고도와 무관하게 고정.
		// 플레이어 머리 위로 띄울 높이. 주변에서 가장 높은 지형/건물보다 높기만 하면 됨.
		// (직교 투영이라 높이를 바꿔도 확대/축소는 변하지 않음. 클리핑 여부만 결정)
	UPROPERTY(EditAnywhere, Category = "Minimap", meta = (ClampMin = "1"))
	float CameraHeight = 7500.f;
	
};