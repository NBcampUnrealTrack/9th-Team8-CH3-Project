//DevTeamWidget.h
	//AI 도움없이 순수 혼자 도전!
	//어차피 SettingWidget 따라치기.
	// 는 실패. 실수로 안 고친 부분 있고, 불필요한 Tick 부분을 안 뺌.


#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DevTeamWidget.generated.h"


class UButton;
class UTextBlock;


UCLASS()
class CH3TEAMP_API UDevTeamWidget : public UUserWidget
{
	GENERATED_BODY()
	
	
protected:
	// 버튼 바인딩은 1회만. (창을 닫았다 다시 열어도 중복 바인딩되지 않게)
	virtual void NativeOnInitialized() override;


	UPROPERTY(meta = (BindWidget))
	UButton* CloseButton;
	
	
private:
	UFUNCTION() void OnCloseClicked();

};
