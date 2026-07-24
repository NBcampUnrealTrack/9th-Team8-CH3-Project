// CH3SettingWidget.h
	// 설정 팝업. 메인메뉴/일시정지 양쪽에서 같은 위젯을 띄움.
	// 좌우에 화살표 버튼 방식. 흔히 보는 그 화살표.
	// 블루프린트만으로도 만들 수 있음.
	// — UButton::OnClicked는 파라미터가 없어서 SlateCore 모듈 의존이 생기지 않음. (Build.cs 수정 불필요)

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CH3SettingWidget.generated.h"

class UButton;
class UTextBlock;

UCLASS()
class CH3TEAMP_API UCH3SettingWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
		// 버튼 바인딩은 1회만. (창을 닫았다 다시 열어도 중복 바인딩되지 않게)
	virtual void NativeOnInitialized() override;

		// 열 때마다 현재 설정을 다시 읽어와 화면에 반영.
	virtual void NativeConstruct() override;

		// WBP 디자이너와 이름으로 연결. 철자가 정확히 일치해야 함.
		// 지금은 알기 쉽게 풀네임 늘여쓰기.
	UPROPERTY(meta = (BindWidget))
	UButton* PrevResolutionButton;

	UPROPERTY(meta = (BindWidget))
	UButton* NextResolutionButton;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ResolutionText;

	UPROPERTY(meta = (BindWidget))
	UButton* PrevWindowModeButton;

	UPROPERTY(meta = (BindWidget))
	UButton* NextWindowModeButton;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* WindowModeText;

	UPROPERTY(meta = (BindWidget))
	UButton* ApplyButton;

	UPROPERTY(meta = (BindWidget))
	UButton* CloseButton;

private:
		// 버튼 OnClicked는 파라미터가 없어서, 어느 버튼인지 함수로 구분.
		// (강화 카드에서 OnCard0/1/2Clicked로 나눈 것과 같은 방식)
	UFUNCTION() void OnPrevResolutionClicked();
	UFUNCTION() void OnNextResolutionClicked();
	UFUNCTION() void OnPrevWindowModeClicked();
	UFUNCTION() void OnNextWindowModeClicked();
	UFUNCTION() void OnApplyClicked();
	UFUNCTION() void OnCloseClicked();

		// 모니터가 지원하는 해상도 목록. 화살표로 이 배열을 앞뒤로 훑음.
		// 해상도 알아내는 ㄱ너 언리얼 엔진 내에서 자체로 지원해주는 내용인듯.
	TArray<FIntPoint> AvailableResolutions;

		// 지금 화면에 "표시 중인" 항목 번호. 적용 버튼을 눌러야 실제로 반영됨.
	int32 SelectedResolutionIndex = 0;

		// 0 = 창 모드, 1 = 전체화면. 2가지뿐이라 배열 없이 숫자로 관리.
	int32 SelectedWindowModeIndex = 0;

	void RefreshResolutionList();
	void UpdateResolutionText();
	void UpdateWindowModeText();
};