// CH3PauseWidget.h
// 역할: 일시정지 메뉴 화면. 버튼 클릭을 받아 컨트롤러/레벨 전환에 위임만 한다.
//		 열기/닫기와 SetGamePaused는 전부 ACH3HUD 소관 — 위젯은 건드리지 않는다.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CH3PauseWidget.generated.h"

class UButton;

UCLASS()
class CH3TEAMP_API UCH3PauseWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	// BindWidget: WBP의 위젯 이름과 철자까지 정확히 같아야 한다. (MainMenu와 동일 규칙)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Resume;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Settings;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_ReturnToMenu;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Quit;

	UFUNCTION() void OnResumeClicked();
	UFUNCTION() void OnSettingsClicked();
	UFUNCTION() void OnReturnToMenuClicked();
	UFUNCTION() void OnQuitClicked();

	// 메인메뉴 레벨 이름. BP에서 "L_MainMenu" 지정. (MainMenuWidget의 GameplayLevelName과 대칭)
	UPROPERTY(EditDefaultsOnly, Category = "CH3|UI")
	FName MainMenuLevelName = NAME_None;

	// 세팅 팝업. WBP_Setting 완성 후 BP에서 지정. 미지정이면 로그만 뜨고 무동작 (MainMenu와 동일 패턴)
	UPROPERTY(EditDefaultsOnly, Category = "CH3|UI")
	TSubclassOf<UUserWidget> SettingWidgetClass;

private:
	UPROPERTY()
	TObjectPtr<UUserWidget> SettingWidget;
};