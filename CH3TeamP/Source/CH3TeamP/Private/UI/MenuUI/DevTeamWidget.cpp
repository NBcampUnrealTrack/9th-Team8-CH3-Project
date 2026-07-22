// DevTeamWidget.cpp


#include "UI/MenuUI/DevTeamWidget.h"
#include "Components/Button.h"				

//최초 1회만 실행되는 초기화. 버튼 연결 담당.
void UDevTeamWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	if (CloseButton)
	{
		CloseButton->OnClicked.AddDynamic(this, &UDevTeamWidget::OnCloseClicked);
	}
}

//------ 닫기 버튼.
void UDevTeamWidget::OnCloseClicked()
{
		//얘가 과연 필요할까?
	RemoveFromParent();
}