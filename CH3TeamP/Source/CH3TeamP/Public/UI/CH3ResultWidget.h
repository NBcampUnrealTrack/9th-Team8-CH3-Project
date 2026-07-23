// CH3ResultWidget.h
// 게임 클리어/오버 시 뜨는 결과 화면. 하나의 클래스로 양쪽을 겸함(문구/배경만 분기).

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CH3ResultWidget.generated.h"

class UTextBlock;
class UButton;
class UImage;

UCLASS()
class CH3TEAMP_API UCH3ResultWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// HUD가 클리어/오버 여부를 알려주며 이 위젯을 띄울 때 호출.
	void ShowResult(bool bIsClear);

protected:
	virtual void NativeOnInitialized() override;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ResultText;

	// 클리어/오버에 따라 배경 이미지를 바꾸고 싶을 때. 없으면 무시됨(Optional).
	UPROPERTY(meta = (BindWidgetOptional))
	UImage* ResultBackground;

	UPROPERTY(EditDefaultsOnly, Category = "CH3|Result")
	UTexture2D* ClearBackgroundTexture;

	UPROPERTY(EditDefaultsOnly, Category = "CH3|Result")
	UTexture2D* GameOverBackgroundTexture;

	UPROPERTY(meta = (BindWidget))
	UButton* RestartButton;

	UPROPERTY(meta = (BindWidget))
	UButton* MainMenuButton;

	UPROPERTY(meta = (BindWidget))
	UButton* QuitButton;

private:
	UFUNCTION() void OnRestartClicked();
	UFUNCTION() void OnMainMenuClicked();
	UFUNCTION() void OnQuitClicked();
};