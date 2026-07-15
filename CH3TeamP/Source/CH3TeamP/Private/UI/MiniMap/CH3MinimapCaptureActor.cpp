// [신규 파일] CH3MinimapCaptureActor.cpp

#include "UI/MiniMap/CH3MinimapCaptureActor.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Kismet/GameplayStatics.h"


ACH3MinimapCaptureActor::ACH3MinimapCaptureActor()
{
		// 플레이어를 따라다녀야 하므로 Tick 사용.
		// (Tick에서 하는 일은 위치 이동뿐 - 촬영 명령은 컴포넌트가 알아서 함)
	PrimaryActorTick.bCanEverTick = true;

	CaptureComponent = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("CaptureComponent"));
	RootComponent = CaptureComponent;

		// ── 핵심 설정 1: 매 프레임 자동 촬영 ──
		// 미니맵 배경이 플레이어 이동을 따라 계속 흘러야 하므로 매 프레임 갱신이 필요.
		// 촬영 범위가 좁고(OrthoWidth 8000) RT 해상도도 작아(512) 비용은 감당 가능한 수준.
	CaptureComponent->bCaptureEveryFrame = true;

		// ── 핵심 설정 2: 직교(Orthographic) 투영 ──
		// 원근(Perspective)으로 찍으면 가장자리가 왜곡되어 마커 좌표 계산이 틀어짐.
		// 직교로 찍어야 "이미지의 픽셀 비율 = 월드 거리 비율"이 정확히 성립.
		// ※ 이 버전 엔진은 OrthoWidth 약 30만 이상에서 직교 캡쳐가 깨지지만(격자무늬),
		//   지금은 8000이라 문제 범위 밖.
	CaptureComponent->ProjectionType = ECameraProjectionMode::Orthographic;
 
		// ── 핵심 설정 3: 캡쳐 결과물 종류 ──
		// SCS_FinalColorLDR = 라이팅/포스트프로세스가 적용된 "눈에 보이는 그대로"의 색.
		// (SCS_SceneColorHDR 등은 톤매핑 전 원본이라 미니맵용으론 어둡거나 물빠져 보임)
	CaptureComponent->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
 
		// 수직으로 내려다보는 각도
	CaptureComponent->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));

}

void ACH3MinimapCaptureActor::BeginPlay()
{
	Super::BeginPlay();
 
	if (CaptureComponent)
	{
		// 에디터에서 조정한 OrthoWidth 값을 컴포넌트에 반영
		CaptureComponent->OrthoWidth = OrthoWidth;
		
		
	}
}
 
void ACH3MinimapCaptureActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
 
	// 플레이어 바로 위를 따라다니며 촬영.
	// → 플레이어는 항상 촬영 중심 = 미니맵 이미지의 정중앙.
	//   (그래서 HUD 쪽 플레이어 마커도 중앙 고정, 배경만 흐르는 구조)
	if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
	{
		FVector Loc = PlayerPawn->GetActorLocation();
			// X/Y만 플레이어를 따라가고, 높이는 절대 고도로 고정.
			// (플레이어 고도를 더하면 산을 오를 때 카메라도 같이 올라가
			//  클리핑/밝기 조건이 매번 바뀌어 미니맵이 출렁이는 문제가 있었음)
		Loc.Z = CameraHeight;
		SetActorLocation(Loc);
	}
}


FVector2D ACH3MinimapCaptureActor::GetMinimapWorldSize() const
{
	// RenderTarget이 정사각형이므로 세로 촬영 범위도 OrthoWidth와 같음.
	// (직사각 RT를 쓰면 세로 = OrthoWidth * RT세로/RT가로 로 따로 계산해야 함)
	return FVector2D(OrthoWidth, OrthoWidth);
}