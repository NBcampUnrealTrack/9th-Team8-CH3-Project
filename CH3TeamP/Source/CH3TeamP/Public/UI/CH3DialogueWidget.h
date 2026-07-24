// CH3DialogueWidget.h
// 웨이브 시작/경고/보스 등장 시 잠깐 뜨는 NPC 대사창.
// 킬피드와 같은 패턴: 생성되면 스스로 타이머를 걸어 일정 시간 후 사라짐.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CH3DialogueWidget.generated.h"

class UTextBlock;
class UImage;

UCLASS()
class CH3TEAMP_API UCH3DialogueWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 표시할 대사와, 초상화를 NPC로 바꿀지 여부를 받아 갱신.
	void ShowDialogue(const FText& Message, bool bUseNPCIcon);

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* DialogueText;

	// 평소엔 PlayerIcon, 대사 뜰 때 NPCIcon으로 교체.
	UPROPERTY(meta = (BindWidget))
	UImage* PortraitImage;

	UPROPERTY(EditAnywhere, Category = "Dialogue")
	UTexture2D* PlayerIconTexture;

	UPROPERTY(EditAnywhere, Category = "Dialogue")
	UTexture2D* NPCIconTexture;

	// 대사창이 떠있는 시간(초).
	UPROPERTY(EditAnywhere, Category = "Dialogue")
	float LifeTime = 4.f;

private:
	FTimerHandle RemoveTimerHandle;

	UFUNCTION()
	void RemoveSelf();
};