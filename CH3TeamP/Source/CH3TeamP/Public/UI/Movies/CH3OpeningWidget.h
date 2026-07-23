#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CH3OpeningWidget.generated.h"

class UMediaPlayer;
class UMediaSource;

UCLASS()
class CH3TEAMP_API UCH3OpeningWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
		//건너뛰기
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	// 재생할 미디어 플레이어. WBP에서 MP_Opening 지정.
	UPROPERTY(EditAnywhere, Category = "Opening")
	UMediaPlayer* MediaPlayer;

	// 재생할 소스. WBP에서 FMS_Opening 지정.
	UPROPERTY(EditAnywhere, Category = "Opening")
	UMediaSource* MediaSource;
	
public:
	// 재생 끝난 뒤 이동할 레벨 이름.
	UPROPERTY(EditAnywhere, Category = "Opening")
	FName NextLevelName = TEXT("PAS_Demo");

private:
	UFUNCTION()
	void HandlePlaybackEnded();
};