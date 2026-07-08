// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/CH3MainHUDWidget.h"
#include "Components/CanvasPanelSlot.h"








/*
void UCH3MainHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 플레이어 블립 미리 하나 생성
	if (PlayerBlipWidgetClass && MinimapBlipCanvas)
	{
		PlayerBlip = CreateWidget<UMiniMapWidget>(this, PlayerBlipWidgetClass);
		if (PlayerBlip)
		{
			MinimapBlipCanvas->AddChildToCanvas(PlayerBlip);
		}
	}
}

void UCH3MainHUDWidget::NativeTick(const FGeometry& MyGeometry, float DeltaTime)
{
	Super::NativeTick(MyGeometry, DeltaTime);

	APawn* PlayerPawn = GetOwningPlayerPawn();
	if (!PlayerPawn || !PlayerBlip) return;

	FVector2D PixelPos = WorldToMinimapPosition(PlayerPawn->GetActorLocation());

	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(PlayerBlip->Slot))
	{
		CanvasSlot->SetPosition(PixelPos);
	}
}

FVector2D UCH3MainHUDWidget::WorldToMinimapPosition(const FVector& WorldLocation)
{
	float NormalizedX = (WorldLocation.X - MinimapWorldOrigin.X) / MinimapWorldSize.X;
	float NormalizedY = (WorldLocation.Y - MinimapWorldOrigin.Y) / MinimapWorldSize.Y;

	// 테스트해보고 위아래/좌우가 뒤집히면 아래 주석 해제
	// NormalizedX = 1.f - NormalizedX;
	// NormalizedY = 1.f - NormalizedY;

	return FVector2D(NormalizedX * MinimapImagePixelSize.X, 
					  NormalizedY * MinimapImagePixelSize.Y);
}
*/

