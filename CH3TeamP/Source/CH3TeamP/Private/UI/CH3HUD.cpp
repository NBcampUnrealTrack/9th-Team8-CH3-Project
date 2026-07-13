// CH3HUD.cpp

#include "UI/CH3HUD.h"
#include "UI/CH3MainHUDWidget.h"

void ACH3HUD::BeginPlay()
{
	Super::BeginPlay();

	if (!MainHUDClass)
	{
			// 디버깅용 : 에디터에서 MainHUDClass를 안 채웠을 때, "조용히 안 뜨는" 대신 바로 알아챌 수 있게 로그를 남기는 용도.
		UE_LOG(LogTemp, Error, TEXT("ACH3HUD: MainHUDClass가 지정되지 않았습니다!"));
		return;
	}

		// GetOwningPlayerController(): 나(HUD)를 소유한 PlayerController. - OwningPla~때문에 PlayerController가 필요했던 거. 그거 대체하기 위해.
		// AHUD는 PlayerController가 자동으로 만들어서 데리고 다니므로 이 값은 항상 유효함.
	MainHUD = CreateWidget<UCH3MainHUDWidget>(GetOwningPlayerController(), MainHUDClass);
	if (MainHUD)
	{
		MainHUD->AddToViewport();
	}
	
}