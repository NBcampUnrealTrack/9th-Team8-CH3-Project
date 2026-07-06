// CH3MapBoundsChecker.cpp
// UI용 디버깅 클래스


#include "Debug/CH3MapBoundsChecker.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

ACH3MapBoundsChecker::ACH3MapBoundsChecker()
{
	PrimaryActorTick.bCanEverTick = false; // 매 프레임 계산할 필요 없음, 한 번만 실행
}

// [신규 추가] 게임 시작 시 자동으로 지형 범위를 계산해서 화면에 출력
void ACH3MapBoundsChecker::BeginPlay()
{
	Super::BeginPlay();

	FBox CombinedBounds(EForceInit::ForceInit); // 빈 박스로 시작

	TArray<AActor*> AllActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);

	for (AActor* Actor : AllActors)
	{
		// 이름에 "Landscape"가 포함된 액터만 필터링
		if (Actor->GetName().Contains(TEXT("Landscape")))
		{
			FVector Origin, BoxExtent;
			Actor->GetActorBounds(false, Origin, BoxExtent);

			CombinedBounds += FBox(Origin - BoxExtent, Origin + BoxExtent);
		}
	}

	// 결과를 화면에 30초간 크게 출력
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 30.f, FColor::Green,
			FString::Printf(TEXT("Map Min: %s"), *CombinedBounds.Min.ToString()));
		GEngine->AddOnScreenDebugMessage(-1, 30.f, FColor::Green,
			FString::Printf(TEXT("Map Max: %s"), *CombinedBounds.Max.ToString()));
	}
}