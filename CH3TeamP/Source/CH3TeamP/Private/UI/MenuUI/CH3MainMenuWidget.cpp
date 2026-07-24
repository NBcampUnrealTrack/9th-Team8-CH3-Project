#include "UI/MenuUI/CH3MainMenuWidget.h"
#include "Components/Button.h"
#include "UI/Movies/CH3OpeningWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

void UCH3MainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// AddDynamic = 동적 델리게이트에 UFUNCTION을 연결하는 매크로.
	if (Btn_GameStart)
	{
		Btn_GameStart->OnClicked.AddDynamic(this, &UCH3MainMenuWidget::OnGameStartClicked);
	}

	if (Btn_Settings)
	{
		Btn_Settings->OnClicked.AddDynamic(this, &UCH3MainMenuWidget::OnSettingsClicked);
	}
	
	if (Btn_DevTeam)
	{
		Btn_DevTeam->OnClicked.AddDynamic(this, &UCH3MainMenuWidget::OnDevTeamClicked);
	}

	if (Btn_Quit)
	{
		Btn_Quit->OnClicked.AddDynamic(this, &UCH3MainMenuWidget::OnQuitClicked);
	}

	if (Btn_LoadGame)
	{
			// 세이브/로드 아직 미구현. 버튼을 지우지 않고 비활성화만. false.
			// 나중에 구현할 때 UMG 레이아웃을 다시 안 짤 필요 없음.
		Btn_LoadGame->SetIsEnabled(false);
	}
}


	//게임 시작 버튼 누르는 거.
void UCH3MainMenuWidget::OnGameStartClicked()
{
	if (GameplayLevelName.IsNone())
	{
		UE_LOG(LogTemp, Error, TEXT("[CH3MainMenu] GameplayLevelName 미지정. WBP Details에서 설정할 것"));
		return;
	} //디버깅.

		// 게임플레이 레벨을 새로 열기.
		// CH3TeamProjectGameMode는 AutoStart BeginPlay가 true인 게 기본값.
		// BeginPlay에서 알아서 StartGame()을 호출하도록. (GameMode 수정 불필요)
	// [변경] 바로 레벨을 열지 않고, 오프닝 영상 위젯을 먼저 띄운다.
	// 영상이 끝나면 CH3OpeningWidget이 GameplayLevelName으로 이동한다.
	if (OpeningWidgetClass)
	{
		if (UCH3OpeningWidget* OpeningWidget = CreateWidget<UCH3OpeningWidget>(GetOwningPlayer(), OpeningWidgetClass))
		{
			OpeningWidget->NextLevelName = GameplayLevelName;
			OpeningWidget->AddToViewport(100);
		}
	}
	else
	{
		// 오프닝 위젯 클래스가 지정 안 됐으면, 예전처럼 바로 레벨 이동 (안전장치).
		UGameplayStatics::OpenLevel(this, GameplayLevelName);
	}}


	//세팅 버튼 누를 시.
void UCH3MainMenuWidget::OnSettingsClicked()
{
	if (!SettingWidgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("[CH3MainMenu] SettingWidgetClass 미지정. WBP Details에서 설정할 것"));
		return;
	}

		// 이미 열려있으면 또 만들지 않음. (버튼 연타 방어)
	if (SettingWidget && SettingWidget->IsInViewport())
	{
		return;
	}

	SettingWidget = CreateWidget<UUserWidget>(GetOwningPlayer(), SettingWidgetClass);
	if (SettingWidget)
	{
		// ZOrder 10 → 메인메뉴(0) 위에 얹힌다. 메인메뉴는 뒤에 그대로 남는다.
		SettingWidget->AddToViewport(10);
	}
}

	// 팀 소개 버튼을 누를 시.
void UCH3MainMenuWidget::OnDevTeamClicked()
{
	if (!DevTeamWidgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("[CH3MainMenu] DevTeamWidgetClass 미지정. WBP Details에서 설정할 것"));
		return;
	}

		// 이미 열려있으면 또 만들지 않음. (버튼 연타 방어)
	if (DevTeamWidget && DevTeamWidget->IsInViewport())
	{
		return;
	}

	DevTeamWidget = CreateWidget<UUserWidget>(GetOwningPlayer(), DevTeamWidgetClass);
	if (DevTeamWidget)
	{
		// ZOrder 10 → 메인메뉴(0) 위에 얹힌다. 메인메뉴는 뒤에 그대로 남는다.
		DevTeamWidget->AddToViewport(10);
	}
}




	// 나가기 버튼 누를 시 -> 종료.
void UCH3MainMenuWidget::OnQuitClicked()
{
		// 에디터 PIE에서는 PIE 종료, 패키징 빌드에서는 프로그램 종료.
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}