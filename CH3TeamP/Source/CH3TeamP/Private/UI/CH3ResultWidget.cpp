// CH3ResultWidget.cpp

#include "UI/CH3ResultWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "CH3TeamProjectGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

void UCH3ResultWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (RestartButton)
	{
		RestartButton->OnClicked.AddDynamic(this, &UCH3ResultWidget::OnRestartClicked);
	}
	if (MainMenuButton)
	{
		MainMenuButton->OnClicked.AddDynamic(this, &UCH3ResultWidget::OnMainMenuClicked);
	}
	if (QuitButton)
	{
		QuitButton->OnClicked.AddDynamic(this, &UCH3ResultWidget::OnQuitClicked);
	}
}

void UCH3ResultWidget::ShowResult(bool bIsClear)
{
	if (ResultText)
	{
		ResultText->SetText(FText::FromString(bIsClear ? TEXT("생존 성공!") : TEXT("YOU DIED")));
	}

	if (ResultBackground)
	{
		UTexture2D* Texture = bIsClear ? ClearBackgroundTexture : GameOverBackgroundTexture;
		if (Texture)
		{
			ResultBackground->SetBrushFromTexture(Texture, false);
		}
	}

	// 결과창은 마우스로 버튼 클릭해야 하니 UI 전용 입력모드로 전환.
	if (APlayerController* PC = GetOwningPlayer())
	{
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(TakeWidget());
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = true;
	}
}

void UCH3ResultWidget::OnRestartClicked()
{
	if (ACH3TeamProjectGameMode* GM = Cast<ACH3TeamProjectGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GM->RestartGame();
	}
}

void UCH3ResultWidget::OnMainMenuClicked()
{
	// 경로는 실제 메인메뉴 맵 경로로 맞춰야 함.
	UGameplayStatics::OpenLevel(this, FName(TEXT("/Game/UI/MainMenu/L_MainMenu")));
}

void UCH3ResultWidget::OnQuitClicked()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		UKismetSystemLibrary::QuitGame(this, PC, EQuitPreference::Quit, false);
	}
}