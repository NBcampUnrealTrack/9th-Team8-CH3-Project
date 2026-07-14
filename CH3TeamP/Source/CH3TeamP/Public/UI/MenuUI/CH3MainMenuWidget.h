#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CH3MainMenuWidget.generated.h"

class UButton;

UCLASS()
class CH3TEAMP_API UCH3MainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
		// 위젯이 만들어져 화면에 붙을 때 1회 호출. 여기서 버튼 클릭을 바인딩.
	virtual void NativeConstruct() override;

		//-------버튼 부분.
		//BindWidget: WBP 안의 위젯을 C++ 변수에 자동 연결.
		// 변수명이 WBP의 위젯 이름과 철자까지 정확히 같아야 한다.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_GameStart;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_LoadGame;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Settings;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Quit;

		// 버튼 콜백.
		// OnClicked는 동적 델리게이트기 때문에 UFUNCTION()이 반드시 필요.
		// 리플렉션 시스템을 거쳐서.
	UFUNCTION()
	void OnGameStartClicked();

	UFUNCTION()
	void OnSettingsClicked();

	UFUNCTION()
	void OnQuitClicked();

		// 에디터에서 지정하는 값
		// 게임 시작 시 열 레벨 이름. BP에서 지정.
	UPROPERTY(EditDefaultsOnly, Category = "CH3|UI")
	FName GameplayLevelName = NAME_None;

		// 세팅 팝업 위젯. BP에서 WBP_Setting을 지정.
	UPROPERTY(EditDefaultsOnly, Category = "CH3|UI")
	TSubclassOf<UUserWidget> SettingWidgetClass;

private:
	UPROPERTY()
	TObjectPtr<UUserWidget> SettingWidget;
};