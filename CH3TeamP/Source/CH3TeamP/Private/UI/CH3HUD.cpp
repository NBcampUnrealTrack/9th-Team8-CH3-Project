// CH3HUD.cpp

#include "UI/CH3HUD.h"
#include "UI/CH3MainHUDWidget.h"
#include "Characters/Player/PlayerCharacter.h" // 경험치 획득 PIE용(7/23)
#include "UI/MenuUI/CH3PauseWidget.h"		// pause 일시정지 UI 위젯 클래스.
#include "UI/Upgrade/CH3UpgradeSelectWidget.h"	// [카드 강화 추가 7/19] 강화 카드 선택 위젯.
#include "UI/Upgrade/CH3UpgradeListWidget.h" // 카드 강화 인벤토리용.


#include "Characters/Player/PlayerCharacter.h"
#include "Components/HealthComponent.h"

#include "CH3TeamProjectGameMode.h"			// [카드 강화 추가 7/19] 강화 방송 구독 + 테스트용 NotifyPlayerLevelUp 호출.
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
		

			// PIE에서는 ESC누르면 실행 종료됨. 그래서 P키 누르면 Pause가 실행되도록.
		FInputKeyBinding& TestBinding = InputComponent->BindKey(
			EKeys::P, IE_Pressed, this, &ACH3HUD::TogglePauseMenu);
		TestBinding.bExecuteWhenPaused = true;
		

			// [추가 7/19] 강화 카드 PIE 테스트용 — 배포 전 삭제할 것.
			// [ (LeftBracket)키 = 강제 카드 발동.
			// [가 LeftBracket이었구나.
			// 여기는 일부러 bExecuteWhenPaused를 안 킴.
			// 카드가 이미 떠서 일시정지된 동안 [을 또 눌러 중복 요청되는 걸 입력 단계에서부터 차단. (게임모드에도 자체 가드가 있어서 이중 안전)
	//	InputComponent->BindKey(EKeys::LeftBracket, IE_Pressed, this, &ACH3HUD::DebugTriggerLevelUp);
		
		

	}

		// -------강화 카드 부분.		// [추가 7/19] 여기부터 새 블록.
		// 게임모드의 두 방송을 구독한다. (카드 제시 = 열기 / 선택 확정 = 닫기)
		// 게임모드는 레벨 로드 시 HUD보다 먼저 만들어지므로 BeginPlay 시점에 항상 잡힌다.
	if (ACH3TeamProjectGameMode* GM = GetWorld()->GetAuthGameMode<ACH3TeamProjectGameMode>())
	{
		GM->OnUpgradeCardsPresented.AddDynamic(this, &ACH3HUD::HandleUpgradeCardsPresented);
		GM->OnUpgradeConfirmed.AddDynamic(this, &ACH3HUD::HandleUpgradeConfirmed);
	}
	else
	{
			// 메뉴 레벨 등 이 게임모드가 아닌 곳에는 강화 카드가 원래 없음. 기록만 남김.
		UE_LOG(LogTemp, Warning, TEXT("ACH3HUD: CH3TeamProjectGameMode가 아니어서 강화 카드 방송에 바인딩하지 않았습니다."));
	}
	
	
	
	InputComponent->BindKey(EKeys::I, IE_Pressed, this, &ACH3HUD::ToggleUpgradeInventory);
	
	
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


	// 하단은 강화 카드 화면 부분.
	// [카드 강화 추가 7/19] 아래 함수 3개 전부 새로 추가.
void ACH3HUD::HandleUpgradeCardsPresented(const TArray<EUpgradeType>& Cards)
{
	if (!UpgradeSelectWidgetClass)
	{
			// PauseWidgetClass 미지정 때와 같은 패턴: 조용히 안 뜨는 대신 바로 알아채게 로그.
		UE_LOG(LogTemp, Error, TEXT("ACH3HUD: UpgradeSelectWidgetClass가 지정되지 않았습니다!"));
		return;
	}

	APlayerController* PC = GetOwningPlayerController();
	if (!PC)
	{
		return;
	}

		// Pause와 동일: 최초 1회만 생성, 그리고 재사용 가능.
	if (!UpgradeSelectWidget)
	{
		UpgradeSelectWidget = CreateWidget<UCH3UpgradeSelectWidget>(PC, UpgradeSelectWidgetClass);
	}
	if (!UpgradeSelectWidget)
	{
		return;
	}

		// 이번에 제시된 카드들로 3칸을 갱신한 뒤 화면에 올림.
	UpgradeSelectWidget->SetupCards(Cards);
	if (!UpgradeSelectWidget->IsInViewport())
	{
		UpgradeSelectWidget->AddToViewport(60);		// Pause(ZOrder50)보다 위층. 상태상 동시에 뜰 일은 없지만 층을 명시적으로 구분.
	}

		// 일시정지는 게임모드가 이미 걸었음(NotifyPlayerLevelUp 안에서).
		// HUD는 강화 흐름에서 SetGamePaused를 절대 만지지 않는다. (푸는 것도 게임모드 담당)
		// 여기서는 "일시정지 중에도 카드 버튼을 클릭할 수 있게" 입력모드만 바꿈. (OpenPauseMenu와 같은 이유로 GameAndUI)
	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	PC->SetInputMode(InputMode);
	PC->bShowMouseCursor = true;
}

void ACH3HUD::HandleUpgradeConfirmed(EUpgradeType ChosenUpgrade, AController* ForPlayer)
{
		// 게임 재개(SetGamePaused(false))는 게임모드(ConfirmUpgradeSelection 안에서). 여기선 화면/입력만 원복.
	if (UpgradeSelectWidget && UpgradeSelectWidget->IsInViewport())
	{
		UpgradeSelectWidget->RemoveFromParent();		// 파괴 아님. Pause와 동일하게 인스턴스 재사용.
	}

	if (APlayerController* PC = GetOwningPlayerController())
	{
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = false;
	}
}


	// I키 입력을 받아 MainHUD 위젯의 창 토글로 위임한다.
	// HUD 액터는 입력만 받고, 실제 창 생성/여닫기는 위젯(UCH3MainHUDWidget)이 처리한다.
void ACH3HUD::ToggleUpgradeInventory()
{
	if (MainHUD)
	{
		MainHUD->ToggleUpgradeInventory();
	}
}

/*
	//------------이 아래는 디버그용.-------------------------
void ACH3HUD::DebugTriggerLevelUp()
{
		// PIE 테스트용 — 배포 전 삭제할 것.
		// 추가 수정 - 7/23 : 레벨업을 경험치 획득으로 변경.
		// 게임모드를 직접 부르는 대신, 캐릭터의 AddEXP로 경험치를 채움.
		// 이러면 실제 레벨업 경로(AddEXP → OnLevelUp → GameMode::NotifyPlayerLevelUp)를
		// 그대로 타므로, 경험치바/레벨업/카드 UI를 한 번에 검증할 수 있음.
	APawn* Pawn = GetOwningPlayerController() ? GetOwningPlayerController()->GetPawn() : nullptr;
	if (APlayerCharacter* PC = Cast<APlayerCharacter>(Pawn))
	{
			// MaxEXP의 20%만큼 채움. 5번 누르면 레벨업 1회 발생.
		PC->AddEXP(PC->MaxEXP * 0.2f);
	}
}

*/