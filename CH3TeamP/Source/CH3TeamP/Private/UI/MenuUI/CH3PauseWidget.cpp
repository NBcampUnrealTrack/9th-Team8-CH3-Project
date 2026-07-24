// CH3PauseWidget.cpp


#include "UI/MenuUI/CH3PauseWidget.h"
#include "UI/CH3HUD.h"						//Resume이 컨트롤러의 닫기 함수를 부르기 위해
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"


void UCH3PauseWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_Resume)
	{
		Btn_Resume->OnClicked.AddDynamic(this, &UCH3PauseWidget::OnResumeClicked);
	}
	if (Btn_Settings)
	{
		Btn_Settings->OnClicked.AddDynamic(this, &UCH3PauseWidget::OnSettingsClicked);
	}
	if (Btn_ReturnToMenu)
	{
		Btn_ReturnToMenu->OnClicked.AddDynamic(this, &UCH3PauseWidget::OnReturnToMenuClicked);
	}
	if (Btn_Quit)
	{
		Btn_Quit->OnClicked.AddDynamic(this, &UCH3PauseWidget::OnQuitClicked);
	}
}

void UCH3PauseWidget::OnResumeClicked()
{
	// 닫기 로직을 직접 하지 않고 HUD에 위임.
	// SetGamePaused(false)를 부르는 곳을 ACH3HUD::ClosePauseMenu() 한 곳으로 일원화하기 위해.
	// GetOwningPlayer()->GetHUD() : 이 위젯을 소유한 PC가 데리고 다니는 HUD를 가져온다.
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (ACH3HUD* HUD = Cast<ACH3HUD>(PC->GetHUD()))
		{
			HUD->ClosePauseMenu();
			return;
		}
	}

	UE_LOG(LogTemp, Error, TEXT("[CH3PauseWidget] ACH3HUD를 찾지 못했습니다. GameMode의 HUD Class 지정 확인!"));
}

void UCH3PauseWidget::OnSettingsClicked()
{
	if (!SettingWidgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("[CH3PauseWidget] SettingWidgetClass 미지정. WBP Details에서 설정할 것"));
		return;
	}

		// 이미 열려있으면 또 만들지 않음. (버튼 연타 방어 — MainMenu와 동일 패턴)
	if (SettingWidget && SettingWidget->IsInViewport())
	{
		return;
	}

	SettingWidget = CreateWidget<UUserWidget>(GetOwningPlayer(), SettingWidgetClass);
	if (SettingWidget)
	{
		SettingWidget->AddToViewport(60);	// 일시정지 메뉴(50)보다 위층
	}
}

void UCH3PauseWidget::OnReturnToMenuClicked()
{
	if (MainMenuLevelName.IsNone())
	{
		UE_LOG(LogTemp, Error, TEXT("[CH3PauseWidget] MainMenuLevelName 미지정. WBP Details에서 설정할 것"));
		return;
	}

		// OpenLevel은 월드를 통째로 새로 만들므로 일시정지 상태가 이월되진 않지만,
		// "내가 켠 일시정지는 내가 끄고 나간다"는 규칙을 지키기 위해 명시적으로 해제.
	UGameplayStatics::SetGamePaused(this, false);
	UGameplayStatics::OpenLevel(this, MainMenuLevelName);
}

void UCH3PauseWidget::OnQuitClicked()
{
		// 에디터 PIE에서는 PIE 종료, 패키징 빌드에서는 프로그램 종료. (MainMenu와 동일)
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}