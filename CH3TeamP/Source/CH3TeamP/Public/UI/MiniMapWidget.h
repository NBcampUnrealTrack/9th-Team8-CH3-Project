// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MiniMapWidget.generated.h"

/**
 * 
 */
UCLASS()
class CH3TEAMP_API UMiniMapWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(meta = (BindWidget))
	class UImage* BlipIcon; // 플레이어나 적 등을 BlipIcon이라는 이름으로 작은 원, 삼각형 등의 이미지로 배치.
};
