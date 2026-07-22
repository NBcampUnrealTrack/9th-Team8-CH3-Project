// CH3UpgradeSelectWidget.h
	// 웨이브 보상 시 뜨는 "강화 카드 선택 화면" 위젯 1개 생성.
	// 카드 1장 = 완성된 이미지 1장이므로, 카드별 위젯 클래스 없음. 그냥 이미지로.
	// 이 위젯 안의 Image 3칸에 텍스처만 갈아끼운다.
	// 생성/표시/입력모드 전환은 CH3HUD에서. 지금 select위젯은 "표시 + 클릭 전달"만.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CH3GameplayTypes.h"		// EUpgradeType.

#include "CH3UpgradeSelectWidget.generated.h"


class UButton; // 클릭 위해서.
class UImage; // 카드 이미지.
class UTexture2D; // 카드 이미지.


UCLASS()
class CH3TEAMP_API UCH3UpgradeSelectWidget : public UUserWidget
{
	GENERATED_BODY()

public:
		// [HUD -> 위젯] 제시된 카드 배열을 받아 3칸에 표시. HUD가 화면에 올리기 직전에 호출.
		// RollUpgradeCards()에 3장 뽑기 내용 있음.
		// 더 많이 뽑으려면 GameMode에서 UpgradeCardCount를 변경.
	void SetupCards(const TArray<EUpgradeType>& InCards);

protected:
		// Actor의 BeginPlay() 역할.
		// Native의 의미 : "C++쪽"이라는 표시. 블프의 Event~ 역할.
		// NativeConstruct는 위젯이 화면에 올라갈 때, 즉 AddToViewport 될 때마다 불림.
		// 거기서 AddDynamic을 하면 닫았다 다시 열 때 클릭이 중복 바인딩으로, 한 번 클릭에 두 번 실행되는 버그 발생.
		// NativeOn"Initialized"는 위젯이 처음 만들어질 때 딱 1회만 호출됨. 이름값.
	virtual void NativeOnInitialized() override;

		//----- WBP와 "이름"으로 자동 연결되는 위젯들. WBP에서 이름이 정확히 일치해야 함.
		// 이름이 다르면 WBP 컴파일 시 에러로 알려줌. (런타임에 몰래 null이 되는 게 아니라서 안전)
	UPROPERTY(meta = (BindWidget))
	UButton* CardButton_0;

	UPROPERTY(meta = (BindWidget))
	UButton* CardButton_1;

	UPROPERTY(meta = (BindWidget))
	UButton* CardButton_2;

	UPROPERTY(meta = (BindWidget))
	UImage* CardImage_0;

	UPROPERTY(meta = (BindWidget))
	UImage* CardImage_1;

	UPROPERTY(meta = (BindWidget))
	UImage* CardImage_2;

		// 강화 종류 -> 카드 이미지 매핑. WBP의 클래스 디폴트에서 9장 전부 채운다.
	UPROPERTY(EditDefaultsOnly, Category = "CH3|Upgrade")
	TMap<EUpgradeType, UTexture2D*> CardTextures;

private:
	// 버튼의 OnClicked는 파라미터가 없어서, "몇 번째가 눌렸는지"를 함수 3개로 구분한다.
	UFUNCTION()
	void OnCard0Clicked();

	UFUNCTION()
	void OnCard1Clicked();

	UFUNCTION()
	void OnCard2Clicked();

	void HandleCardClicked(int32 CardIndex);

		// 현재 화면에 제시 중인 카드들. 클릭 시 여기서 실제 EUpgradeType을 꺼내 GameMode로 보낸다.
	TArray<EUpgradeType> PresentedCards;
};