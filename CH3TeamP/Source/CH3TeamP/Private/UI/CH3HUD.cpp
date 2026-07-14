// CH3HUD.cpp

#include "UI/CH3HUD.h"
#include "UI/CH3MainHUDWidget.h"
#include "UI/MenuUI/CH3PauseWidget.h"		// pause 일시정지 UI 위젯 클래스.
#include "CH3GameState.h"					// GetPlayState() 게이트용. 현재 게임 상태 확인해서 일시정지 열어도 될지 물을 때.
#include "Components/InputComponent.h"		// 사용자 입력 바인딩. BindKey, BindAction 등등. UE5 EIC니까 BindKey 사용.
#include "Kismet/GameplayStatics.h"			// 일시정지 세팅. 게임 일시정지 기능 제어 위해. SetGamePaused. 
#include "GameFramework/PlayerController.h"	// 인풋모드 세팅. 입력 모드 설정(SetInputMode) 및 마우스 커서를 보이게 하는 기능.

void ACH3HUD::BeginPlay()
{
	Super::BeginPlay();

	if (!MainHUDClass)
	{
			// 디버깅용 : 에디터에서 MainHUDClass를 안 채웠을 때, "조용히 안 뜨는" 대신 바로 알아챌 수 있게 로그를 남기는 용도.
		UE_LOG(LogTemp, Error, TEXT("ACH3HUD: MainHUDClass가 지정되지 않았습니다!"));
		return;
	}

		// GetOwningPlayerController(): 나(HUD)를 소유한 PlayerController.
		// AHUD는 PlayerController가 자동으로 만들어서 데리고 다니므로 이 값은 항상 유효함.
	APlayerController* PC = GetOwningPlayerController();

	MainHUD = CreateWidget<UCH3MainHUDWidget>(PC, MainHUDClass);
	if (MainHUD)
	{
		MainHUD->AddToViewport();
	}
	
	
		//-----게임 입력 모드로 강제 복구
		// 메인메뉴(ACH3MenuGameMode)에서 SetInputMode(FInputModeUIOnly)를 걸어놓고 왔는데,
		// 그 설정이 GameViewportClient에 남아 레벨을 넘어와 버림.
		// (PlayerController는 새로 생성되지만, GameViewportClient는 GameInstance 소속이라 파괴되지 않음.)
		// 그대로 두면 뷰포트가 마우스를 잡지 않아 캐릭터 조작이 안 됨(경험담).
		// ACH3HUD는 게임플레이 레벨에서만 생성되므로, "게임 화면에 진입했다"를 알림.
	if (PC)
	{
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = false;
	}

		// -------일시정지 부분.
		// AHUD는 AActor이므로 EnableInput()을 부르면 자기 InputComponent가 생기고,
		// 그게 PlayerController의 입력 스택에 올라간다. 커스텀 PC를 만들 필요가 없는 이유가 이것.
		// 간단하게, 일시정지 바인딩한 거.
	EnableInput(PC);

	if (InputComponent)
	{
			// Enhanced Input(IMC/IA)을 거치지 않고 키를 직접 바인딩.
			// 이유: IA_Pause/IMC는 에디터 에셋 + 캐릭터 담당 영역이라서.
			// 내것만 건들 거.
			// 반환값 FInputKeyBinding&를 받아 bExecuteWhenPaused를 켜는 게 핵심.
			// 이걸 안 켜면 게임이 일시정지된 순간 이 바인딩이 실행되지 않아서 메뉴는 열리지만, ESC로 닫을 수가 없음.
		FInputKeyBinding& EscBinding = InputComponent->BindKey(
			EKeys::Escape, IE_Pressed, this, &ACH3HUD::TogglePauseMenu);
		EscBinding.bExecuteWhenPaused = true;

			// *****PIE 테스트용 — 배포 전 삭제할 것.*****
			// 에디터 PIE에서는 ESC가 "플레이 중지" 단축키에 먹히게 되니까. 키가 중복돼서 되는지 모름.
			// 그래서 에디터 테스트는 P키로 한다.
		FInputKeyBinding& TestBinding = InputComponent->BindKey(
			EKeys::P, IE_Pressed, this, &ACH3HUD::TogglePauseMenu);
		TestBinding.bExecuteWhenPaused = true;
	}
}



	//------쭉 Pause 메뉴.
void ACH3HUD::TogglePauseMenu()
{
		// 여기서 나오는 게이트 : 
		// 이미 열려 있으면 -> 닫기. (게이트 재검사 불필요: 열려 있다는 건 이미 통과했다는 뜻)
	if (PauseWidget && PauseWidget->IsInViewport())
	{
		ClosePauseMenu();
		return;
	}

		// 닫혀 있으면 -> 게이트를 통과할 때만 연다.
	if (CanOpenPauseMenu())
	{
		OpenPauseMenu();
	}
		// 게이트에 막히면 조용히 무시. (레벨업 카드 선택 중 ESC 연타 = 아무 일도 안 일어남. 그게 정답)
}

bool ACH3HUD::CanOpenPauseMenu() const
{
	const ACH3GameState* GS = GetWorld() ? GetWorld()->GetGameState<ACH3GameState>() : nullptr;
	if (!GS)
	{
		return false;
	}

		// 화이트리스트 방식: "이 두 상태에서만 연다".
		// LevelUpPause만 콕 집어 막는 게 아니라 허용 상태를 명시하는 이유:
		// MainMenu/Starting/GameOver/GameClear에서 열리는 것도 전부 오동작이기 때문.
	const EGamePlayState State = GS->GetPlayState();
	return State == EGamePlayState::WaveInProgress
		|| State == EGamePlayState::WaveIntermission;
}


	//열기.
void ACH3HUD::OpenPauseMenu()
{
	if (!PauseWidgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("ACH3HUD: PauseWidgetClass가 지정되지 않았습니다!"));
		return;
	}

	APlayerController* PC = GetOwningPlayerController();
	if (!PC)
	{
		return;
	}

		// 최초 1회만 생성. 닫을 때 파괴하지 않고 화면에서만 내리므로 이후엔 재사용.
	if (!PauseWidget)
	{
		PauseWidget = CreateWidget<UCH3PauseWidget>(PC, PauseWidgetClass);
	}
	if (!PauseWidget)
	{
		return;
	}

	PauseWidget->AddToViewport(50);		// ZOrder 50 → MainHUD(기본 0)보다 확실히 위층

	UGameplayStatics::SetGamePaused(this, true);

		// UIOnly가 아니라 GameAndUI를 쓰는 이유:
		// UIOnly는 키 입력이 게임(입력 스택)까지 안 내려와서, 위에서 바인딩한 ESC 닫기가 죽는다.
		// GameAndUI면 버튼 클릭도 되고 ESC도 살아있다.
		// (일시정지 중이라 사격/이동은 어차피 실행 안 됨 — 그쪽엔 bExecuteWhenPaused가 없으니까)
	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	PC->SetInputMode(InputMode);
	PC->bShowMouseCursor = true;
}


	//닫기.
void ACH3HUD::ClosePauseMenu()
{
		// 이 가드가 이 시스템에서 가장 중요한 한 줄.
		// 메뉴가 실제로 열려 있지 않으면 아무것도 하지 않는다.
		// 여기서 무조건 SetGamePaused(false)를 해버리면, 팀원의 레벨업 일시정지
		// (참조 카운트 없는 단일 플래그)를 실수로 풀어서 → 카드 미선택 상태로 게임 재개
		// → 스폰 영구 정지가 나기 때문.
	if (!PauseWidget || !PauseWidget->IsInViewport())
	{
		return;
	}

	PauseWidget->RemoveFromParent();		// 파괴가 아님. 화면에서만 제거. 인스턴스 재사용할 거임.

	UGameplayStatics::SetGamePaused(this, false);

	if (APlayerController* PC = GetOwningPlayerController())
	{
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = false;
	}
}