#include "UI/MenuUI/CH3MenuGameMode.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"

ACH3MenuGameMode::ACH3MenuGameMode()
{
		// 메뉴 레벨엔 플레이어 캐릭터가 필요 없음.
		// 비워두지 않으면 빈 레벨에 기본 폰이 떠다니게 됨.
	DefaultPawnClass = nullptr;
}

void ACH3MenuGameMode::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC || !MainMenuWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[CH3MenuGameMode] PC 또는 MainMenuWidgetClass 없음"));
		return;
	}

	MainMenuWidget = CreateWidget<UUserWidget>(PC, MainMenuWidgetClass);
	if (MainMenuWidget)
	{
		MainMenuWidget->AddToViewport(0);		// ZOrder 0 = 가장 아래층
	}

		// 마우스로 버튼을 눌러야 하므로 입력을 UI 전용으로.
	FInputModeUIOnly InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	PC->SetInputMode(InputMode);
	PC->bShowMouseCursor = true;
}