// CH3SettingWidget.cpp
	// 주석이 많은 이유 : 내용 해설도 알기 위해서.

#include "UI/MenuUI/CH3SettingWidget.h"
#include "Components/Button.h"					// UButton 실체. 헤더에선 전방선언만 했으므로 여기서 진짜 정의를 가져옴.
#include "Components/TextBlock.h"				// UTextBlock 실체. SetText()를 쓰려면 필요.
#include "GameFramework/GameUserSettings.h"		// 해상도/창모드 적용·저장 (Engine 모듈)
#include "Kismet/KismetSystemLibrary.h"			// 지원 해상도 목록 조회


	//최초 1회만 실행되는 초기화. 버튼 연결 담당.
void UCH3SettingWidget::NativeOnInitialized()
{
		// 부모(UUserWidget)가 원래 하던 준비 작업을 먼저 실행. 빼먹으면 내부 초기화가 안 됨.
	Super::NativeOnInitialized();

		// 전부 "버튼 누르면 함수 실행해줘." 뜻.
		// AddDynamic은 등록만. 지금 함수를 실행은 아님.
		
		// Construct 대신 Initial~ 쓰는 이유:
		// ~Construct는 창을 열 때마다 실행 + AddDynamic을 하면 창을 두 번째 또 열 때 등록이 2개가 되어 클릭 한 번에 함수가 두 번 실행됨.
		// Initialized는 위젯이 만들어질 때 딱 1회만 실행. 그래서 문제 X.
		
		// if로 감싸는 이유: BindWidget이라 WBP에 이름이 없으면 컴파일 단계에서 걸러지지만,
		// 포인터는 쓰기 전에 확인하는 습관을 유지. (Min의 다른 위젯들과 같은 방식)
	if (PrevResolutionButton)
	{
		PrevResolutionButton->OnClicked.AddDynamic(this, &UCH3SettingWidget::OnPrevResolutionClicked);
	}
	if (NextResolutionButton)
	{
		NextResolutionButton->OnClicked.AddDynamic(this, &UCH3SettingWidget::OnNextResolutionClicked);
	}
	if (PrevWindowModeButton)
	{
		PrevWindowModeButton->OnClicked.AddDynamic(this, &UCH3SettingWidget::OnPrevWindowModeClicked);
	}
	if (NextWindowModeButton)
	{
		NextWindowModeButton->OnClicked.AddDynamic(this, &UCH3SettingWidget::OnNextWindowModeClicked);
	}
	if (ApplyButton)
	{
		ApplyButton->OnClicked.AddDynamic(this, &UCH3SettingWidget::OnApplyClicked);
	}
	if (CloseButton)
	{
		CloseButton->OnClicked.AddDynamic(this, &UCH3SettingWidget::OnCloseClicked);
	}
}


	//------ 화면에 올라올 때마다 실행. 표시 내용 갱신 담당.
void UCH3SettingWidget::NativeConstruct()
{
	Super::NativeConstruct();

		// 이 위젯은 닫아도 파괴하지 않고 화면에서만 내리는 방식(재사용)이라,
		// 이전에 열었을 때의 표시가 그대로 남아 있음.
		// 그래서 열 때마다 "실제 현재 설정"을 다시 읽어와 화면을 맞춰줘야 함.
	RefreshResolutionList();

		// if 괄호 안에서 변수를 선언하는 문법:
		// GetGameUserSettings()의 결과를 Settings에 담고, 그게 nullptr이 아니면 중괄호 안을 실행.
		// Settings는 이 if 블록 안에서만 살아있음. (밖에서 실수로 쓰는 걸 막아줌)
		//
		// GetGameUserSettings()는 static 함수라 객체 없이 바로 호출 가능.
		// 게임 전체에 설정 객체가 딱 하나만 존재하고, 엔진이 알아서 만들어 들고 있음.
	if (UGameUserSettings* Settings = UGameUserSettings::GetGameUserSettings())
	{
			// 삼항 연산자: (조건) ? 참일때값 : 거짓일때값
			// 즉 "현재 창 모드면 0, 아니면 1을 SelectedWindowModeIndex에 넣어라".
			//
			// EWindowMode에는 Windowed / Fullscreen / WindowedFullscreen 3가지가 있는데,
			// 뒤의 둘은 사용자에겐 똑같이 "전체화면"으로 보이므로 하나로 묶어서 1번 처리.
		SelectedWindowModeIndex = (Settings->GetFullscreenMode() == EWindowMode::Windowed) ? 0 : 1;
	}

		// 위에서 정한 번호들을 실제 화면 글자로 반영.
	UpdateResolutionText();
	UpdateWindowModeText();
}


	//------ 모니터가 지원하는 해상도 목록을 받아오고, 시작 위치를 정함.
void UCH3SettingWidget::RefreshResolutionList()
{
		// 이전에 담아둔 내용을 비움.
		// 안 비우면 창을 열 때마다 같은 해상도가 계속 뒤에 쌓여서 목록이 두 배씩 늘어남.
	AvailableResolutions.Empty();

		// 엔진이 모니터를 조사해서 지원 해상도를 알려주는 함수.
		// 특이한 점: 결과를 return으로 돌려주는 게 아니라, 넘긴 배열 안에 직접 채워 넣음.
		// (이런 방식을 "출력 인자"라고 함. 언리얼에서 배열을 돌려줄 때 흔히 쓰는 형태)
		//
		// 하드코딩하지 않는 이유: 모니터마다 지원 해상도가 달라서,
		// 지원 안 하는 값을 넣으면 화면이 안 나오거나 깨질 수 있음.
	UKismetSystemLibrary::GetSupportedFullscreenResolutions(AvailableResolutions);

		// 목록이 비었으면 이후 코드가 전부 의미 없으므로 여기서 중단.
		// 조용히 아무 일도 안 일어나는 대신 로그로 바로 알아채게 함. (기존 위젯들과 같은 방식)
	if (AvailableResolutions.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("[CH3SettingWidget] 지원 해상도 목록을 가져오지 못했습니다."));
		return;
	}

		// 창을 열었을 때 "지금 쓰는 해상도"가 표시돼 있어야 자연스러움.
		// 그래서 현재 해상도가 목록의 몇 번째인지 찾아 시작 위치로 삼는다.
	if (UGameUserSettings* Settings = UGameUserSettings::GetGameUserSettings())
	{
			// const: 이 변수는 아래에서 값이 바뀌지 않는다는 표시. 실수로 바꾸면 컴파일 에러로 잡아줌.
		const FIntPoint Current = Settings->GetScreenResolution();

			// IndexOfByKey: 배열에서 그 값이 몇 번째에 있는지 찾아줌.
			// 못 찾으면 INDEX_NONE(= -1)을 돌려줌.
		const int32 FoundIndex = AvailableResolutions.IndexOfByKey(Current);

			// -1을 그대로 배열 번호로 쓰면 없는 자리를 읽어서 크래시가 남.
			// 그래서 못 찾았을 때는 0번(첫 항목)으로 대체.
			// 못 찾는 경우 예: 에디터 PIE 창 크기처럼 모니터 규격에 없는 해상도로 돌고 있을 때.
		SelectedResolutionIndex = (FoundIndex != INDEX_NONE) ? FoundIndex : 0;
	}
}


	//------ 현재 선택된 해상도를 화면 글자로 표시.
void UCH3SettingWidget::UpdateResolutionText()
{
		// 두 가지를 한 번에 확인:
		// 1) 텍스트 위젯이 있는지  2) 지금 번호가 배열 범위 안인지
		// IsValidIndex는 "0 이상이고 개수보다 작은지"를 검사해줌. 직접 비교하는 것보다 안전.
	if (!ResolutionText || !AvailableResolutions.IsValidIndex(SelectedResolutionIndex))
	{
		return;
	}

		// FIntPoint는 정수 두 개(X, Y)를 담는 구조체. 해상도 표현에 딱 맞음.
	const FIntPoint Res = AvailableResolutions[SelectedResolutionIndex];

		// 글자를 만드는 과정이 3단계라 처음 보면 복잡해 보임:
		// ① TEXT("...")     — 언리얼용 문자열로 만드는 매크로 (한글 등 유니코드 처리를 위해 필수)
		// ② FString::Printf — %d 자리에 숫자를 끼워 넣어 "1920 x 1080" 문자열 완성
		// ③ FText::FromString — UI 표시용 타입으로 변환
		//
		// UI가 FString이 아니라 FText를 쓰는 이유: FText는 다국어 번역 기능을 내장하고 있어서,
		// 나중에 영어판을 만들 때 텍스트만 교체할 수 있는 구조이기 때문.
	ResolutionText->SetText(FText::FromString(
		FString::Printf(TEXT("%d x %d"), Res.X, Res.Y)));
}


	//------ 현재 선택된 창 모드를 화면 글자로 표시.
void UCH3SettingWidget::UpdateWindowModeText()
{
	if (!WindowModeText)
	{
		return;
	}

		// 항목이 2개뿐이라 배열 없이 삼항 연산자로 처리.
		// 여기는 숫자를 끼워 넣을 게 없어서 Printf 없이 TEXT()를 바로 변환.
	WindowModeText->SetText(FText::FromString(
		SelectedWindowModeIndex == 0 ? TEXT("창 모드") : TEXT("전체화면")));
}


	//------ 해상도 왼쪽 화살표. 표시만 바꾸고 실제 적용은 안 함.
void UCH3SettingWidget::OnPrevResolutionClicked()
{
		// 목록이 비었으면 나눗셈에서 0으로 나누게 되어 크래시. 반드시 먼저 막아야 함.
	if (AvailableResolutions.Num() == 0)
	{
		return;
	}

		// % 는 나머지 연산. 5 % 3 = 2.
		// 이걸 쓰면 번호가 끝에 닿았을 때 자동으로 처음으로 돌아옴(순환).
		//
		// 그냥 (번호 - 1) % 개수 로 쓰면 안 되는 이유:
		// 0번에서 1을 빼면 -1이 되는데, C++에서 음수 나머지는 음수라 -1 % 5 = -1.
		// 배열 번호가 -1이 되면 크래시.
		// 그래서 개수를 미리 더해 양수로 만든 뒤 나머지를 구함. (0 - 1 + 5) % 5 = 4 → 맨 끝
	SelectedResolutionIndex = (SelectedResolutionIndex - 1 + AvailableResolutions.Num()) % AvailableResolutions.Num();

		// 화면 글자만 갱신. 실제 해상도는 적용 버튼을 눌러야 바뀜.
		// 화살표마다 바로 적용하면 목록을 훑는 동안 화면이 계속 깜빡이고 위험함.
	UpdateResolutionText();
}


	//------ 해상도 오른쪽 화살표.
void UCH3SettingWidget::OnNextResolutionClicked()
{
	if (AvailableResolutions.Num() == 0)
	{
		return;
	}

		// 이쪽은 더하는 방향이라 음수가 될 일이 없어 그대로 나머지 연산.
		// 맨 끝(개수-1)에서 1을 더하면 개수가 되고, 개수 % 개수 = 0 → 처음으로 순환.
	SelectedResolutionIndex = (SelectedResolutionIndex + 1) % AvailableResolutions.Num();
	UpdateResolutionText();
}


	//------ 창 모드 왼쪽 화살표.
void UCH3SettingWidget::OnPrevWindowModeClicked()
{
		// 항목이 2개뿐이라 좌우 어느 쪽을 눌러도 결과가 같음(0↔1 토글).
		// 그래도 함수를 둘로 나눠둔 건, 나중에 "테두리 없는 전체화면"을 별도 항목으로
		// 추가해 3개가 되면 방향이 달라져야 하기 때문. 그때 이 함수만 고치면 됨.
	SelectedWindowModeIndex = (SelectedWindowModeIndex + 1) % 2;
	UpdateWindowModeText();
}


	//------ 창 모드 오른쪽 화살표.
void UCH3SettingWidget::OnNextWindowModeClicked()
{
	SelectedWindowModeIndex = (SelectedWindowModeIndex + 1) % 2;
	UpdateWindowModeText();
}


	//------ 적용 버튼. 여기서만 실제로 설정이 바뀜.
void UCH3SettingWidget::OnApplyClicked()
{
	UGameUserSettings* Settings = UGameUserSettings::GetGameUserSettings();
	if (!Settings)
	{
		return;
	}

		// 범위 확인 후 적용. 목록을 못 받아온 상황에서 눌려도 크래시하지 않게.
	if (AvailableResolutions.IsValidIndex(SelectedResolutionIndex))
	{
		Settings->SetScreenResolution(AvailableResolutions[SelectedResolutionIndex]);
	}

		// WindowedFullscreen(테두리 없는 전체화면)을 쓰는 이유:
		// 진짜 Fullscreen은 모니터를 독점해서 알트탭 전환이 느리고 멀티모니터에서 불편함.
		// 요즘 게임 대부분이 이쪽을 기본으로 씀.
	Settings->SetFullscreenMode(
		SelectedWindowModeIndex == 0 ? EWindowMode::Windowed : EWindowMode::WindowedFullscreen);

		// 위의 Set 함수들은 "값을 적어두기"만 함. 실제 화면에 반영하려면 이걸 불러야 함.
		// false = 지금 즉시 적용. true는 "다음 실행 시 적용"이라 눌러도 아무 변화가 없어 보임.
	Settings->ApplySettings(false);

		// 게임을 껐다 켜도 유지되도록 파일에 기록.
		// 저장 위치: %LOCALAPPDATA%\CH3TeamP\Saved\Config\Windows\GameUserSettings.ini
		// 이상한 값이 저장돼 화면이 깨지면 이 파일을 지우면 기본값으로 초기화됨.
		// (각자 PC에만 있는 파일이라 팀원에게 전파되지 않음)
	Settings->SaveSettings();
}


	//------ 닫기 버튼.
void UCH3SettingWidget::OnCloseClicked()
{
		// RemoveFromParent는 파괴가 아니라 "화면에서 내리기".
		// 이 위젯을 띄운 쪽(PauseWidget / MainMenuWidget)이 인스턴스를 계속 들고 있으므로,
		// 다음에 열 때 다시 만들지 않고 그대로 재사용함. (일시정지·강화 카드와 같은 방식)
	RemoveFromParent();
}