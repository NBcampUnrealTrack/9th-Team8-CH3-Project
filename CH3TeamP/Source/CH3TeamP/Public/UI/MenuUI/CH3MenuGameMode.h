#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CH3MenuGameMode.generated.h"

class UUserWidget;

	// L_MainMenu 레벨 전용 GameMode.
	// CH3TeamProjectGameMode와는 완전히 별개인 새 클래스.
	// 하는 일: 메인메뉴 위젯을 띄우고, 마우스 입력을 킴.
UCLASS()
class CH3TEAMP_API ACH3MenuGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ACH3MenuGameMode();

protected:
	virtual void BeginPlay() override;

		// 띄울 위젯 클래스. BP에서 WBP_MainMenu를 지정.
		// C++에 에셋 경로를 하드코딩하지 않는 이유 : 에셋 이름이 바뀌면 C++이 깨지기 때문.
	UPROPERTY(EditDefaultsOnly, Category = "CH3|UI")
	TSubclassOf<UUserWidget> MainMenuWidgetClass;

private:
		// UPROPERTY()를 붙여야 GC가 이 위젯을 수거하지 않으니까.
	UPROPERTY()
	TObjectPtr<UUserWidget> MainMenuWidget;
};