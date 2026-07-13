// 메인위젯.

             

//위젯 넣는 아이콘(?)들.
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/VerticalBox.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "TimerManager.h"

//위젯 연결 관련
#include "UI/CH3MainHUDWidget.h"
#include "UI/MiniMap/MiniMapWidget.h"
#include "UI/MiniMap/CH3MinimapCaptureActor.h"
#include "UI/DamageWidget/CH3DamageNumberWidget.h"
#include "UI/DamageWidget/CH3KillFeedEntryWidget.h"
#include "CH3GameState.h"   // GameState 바인딩을 위해 include.

//컴포넌트 및 캐릭터
#include "Characters/Player/PlayerCharacter.h"
#include "Components/HealthComponent.h"
#include "Components/StaminaComponent.h"





	//생명주기 / 델리게이트 바인딩 / 폴링

void UCH3MainHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	SyncMinimapRangeFromCaptureActor();

	
		// 1) 플레이어 마커 생성
	if (PlayerMarkerWidgetClass && MinimapMarkerCanvas)
	{
		PlayerMarker = CreateWidget<UCH3MinimapMarkerWidget>(this, PlayerMarkerWidgetClass);
		
		
		
		if (PlayerMarker)
		{
			MinimapMarkerCanvas->AddChildToCanvas(PlayerMarker);
			if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(PlayerMarker->Slot))
			{
				CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f)); // 보류 중이던 정렬 수정, 여기서 적용
				CanvasSlot->SetPosition(MinimapImagePixelSize * 0.5f); // 정중앙 고정
			}
		}
	}
	
	
	
		//2) 히트마커 이미지
	if (HitMarkerImage)
	{
		HitMarkerImage->SetVisibility(ESlateVisibility::Collapsed);
	}
	
	

	// 3) GameState 델리게이트 바인드(연결) + 초기값 동기화
	BindDelegates();
}




	//NativeTick = Tick. UMG에서 블루프린트에서 사용되는 EventTick과 겹치지 않도록 Native(C++ 네이티브 코드용)접두사 붙임.
	// 위젯에선 1. 크로스헤어 벌리는 거, 2. 미니맵 용. 두 가지로 쓰임.
void UCH3MainHUDWidget::NativeTick(const FGeometry& MyGeometry, float DeltaTime)
{
	UWorld* World = GetWorld();
	if (!World || World->bIsTearingDown)
	{
		return; // 월드가 정리 중이면 아무것도 건드리지 않고 즉시 종료
	}
	
	Super::NativeTick(MyGeometry, DeltaTime);
	
	PollPlayerStatus();
	
		// 크로스헤어 벌어짐 갱신
	UpdateCrosshair(DeltaTime);
	

	
		// ---- 미니맵: 플레이어 마커 위치 갱신 ----
	APawn* PlayerPawn = GetOwningPlayerPawn();
	if (PlayerPawn && PlayerMarker)
	{
		// 위치는 중앙 고정. 방향만 갱신. 마커가 바라보는 방향을 화살표로 (다른 아이콘(원형 점 등)만 쓸 거면 이 줄은 없어도 됨)
		PlayerMarker->SetMarkerAngle(PlayerPawn->GetActorRotation().Yaw);
	}
	
	
}




	// 델리게이트 함수들 바인딩.
void UCH3MainHUDWidget::BindDelegates()
{
		// 유탁님의 유물 : GameState 델리게이트에 "진짜 바인딩"
	ACH3GameState* GS = GetWorld() ? GetWorld()->GetGameState<ACH3GameState>() : nullptr;
	if (!GS)
	{
		// GameState가 연결이 안 되었으면 바인딩 불가. 로그만 남김.
		UE_LOG(LogTemp, Warning, TEXT("CH3MainHUDWidget: GameState를 찾지 못해 바인딩 실패"));
		return;
	}
		//원본 : GameState.cpp
	GS->OnGamePlayStateChanged.AddDynamic(this, &UCH3MainHUDWidget::HandleGamePlayStateChanged);
	GS->OnScoreChanged.AddDynamic(this, &UCH3MainHUDWidget::HandleScoreChanged);
	GS->OnWaveChanged.AddDynamic(this, &UCH3MainHUDWidget::HandleWaveChanged);
	GS->OnEnemiesRemainingChanged.AddDynamic(this, &UCH3MainHUDWidget::HandleEnemiesRemainingChanged);
	GS->OnWaveTimeChanged.AddDynamic(this, &UCH3MainHUDWidget::HandleWaveTimeChanged);

	
	// [중요] 초기값 동기화:
	// 델리게이트는 "값이 바뀌는 순간"만 알려줌. HUD가 생성되기 전에 이미 바뀐 값들은 알림을 못 받았음.
	// 그래서 Getter로 현재값을 한 번 직접 읽어와서 화면을 맞춰준다.
	HandleGamePlayStateChanged(GS->GetPlayState());
	HandleScoreChanged(GS->GetTotalScore());
	HandleWaveChanged(GS->GetCurrentWave(), GS->GetTotalWaves());
	HandleEnemiesRemainingChanged(GS->GetEnemiesRemaining());
	HandleWaveTimeChanged(GS->GetWaveTimeRemaining());
	

	// [TODO - 캐릭터 담당자 코드 완성 후 봉인해제]
	// 캐릭터에 OnHealthChanged / OnStaminaChanged 델리게이트가 생기면 여기서 바인딩:
	// if (APlayerCharacter* PC = Cast<APlayerCharacter>(GetOwningPlayerPawn()))
	// {
	//     PC->OnHealthChanged.AddDynamic(this, &UCH3MainHUDWidget::HandleHealthChanged);
	//     PC->OnStaminaChanged.AddDynamic(this, &UCH3MainHUDWidget::HandleStaminaChanged);
	// }

	// [TODO - 레벨업 카드 UI 만들 때 열 것들.]
	// GameMode의 OnUpgradeCardsPresented에 바인딩 → 카드 UI 표시
	// 선택 완료 시 GameMode->ConfirmUpgradeSelection(선택한카드) 호출
	
	
	// ※ 체력/스태미나는 PollPlayerStatus()가 매 프레임 담당. 아래 HandleHealthChanged/
	//   HandleStaminaChanged는 팀원이 나중에 델리게이트를 추가할 때를 위한 "대기 중" 구현.
 
	// [TODO] PlayerCharacter에 FOnHitConfirmed 델리게이트 추가되면:
	// if (APlayerCharacter* PC = Cast<APlayerCharacter>(GetOwningPlayerPawn()))
	// {
	//     PC->OnHitConfirmed.AddDynamic(this, &UCH3MainHUDWidget::HandleHitConfirmed);
	// }
 
	// [TODO] 탄약/탄환 시스템 생기면:
	// PC->OnAmmoChanged.AddDynamic(this, &UCH3MainHUDWidget::HandleAmmoChanged);
	// PC->OnBulletSpeciesChanged.AddDynamic(this, &UCH3MainHUDWidget::HandleBulletSpeciesChanged);
 
	// [TODO] 데미지/킬 델리게이트가 GameState/GameMode에 생기면:
	// GS->OnDamageDealt.AddDynamic(this, &UCH3MainHUDWidget::HandleDamageDealt);
	// GS->OnEnemyKilled.AddDynamic(this, &UCH3MainHUDWidget::HandleEnemyKilled);
}



	//-----------미니맵 캡쳐 범위
void UCH3MainHUDWidget::SyncMinimapRangeFromCaptureActor()
{
	// NativeConstruct(1회)에서만 호출됨 — NativeTick 안에서 이런 전체 탐색을 하면 안 됨.
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACH3MinimapCaptureActor::StaticClass(), FoundActors);
 
	if (FoundActors.Num() > 0)
	{
		if (ACH3MinimapCaptureActor* CaptureActor = Cast<ACH3MinimapCaptureActor>(FoundActors[0]))
		{
			MinimapWorldSize = CaptureActor->GetMinimapWorldSize();
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("CH3MainHUDWidget: MinimapCaptureActor 없음. .h 폴백 좌표 사용."));
	}
}


	//체력/ 스태미나를 매 프레임 읽어오기.
void UCH3MainHUDWidget::PollPlayerStatus()
{
	if (!CachedPlayer)
	{
		CachedPlayer = Cast<APlayerCharacter>(GetOwningPlayerPawn());
		if (!CachedPlayer)
		{
			return;
		}
	}
 
	if (HealthBar && CachedPlayer->HealthComp)
	{
		HealthBar->SetPercent(CachedPlayer->HealthComp->GetHealthPercent());
	}
 
	if (StaminaBar && CachedPlayer->StaminaComp)
	{
		StaminaBar->SetPercent(CachedPlayer->StaminaComp->GetStaminaPercent());
	}
	
}

		//---------퀘스트 관련(묶음)
void UCH3MainHUDWidget::RefreshQuestDetails()
{
	if (Quest_details)
	{
		// 남은시간과 남은적은 서로 다른 델리게이트로 따로 도착함.
		// 캐시해둔 두 값을 합쳐서 한 줄로 표시. (이벤트 도착 순서와 무관하게 항상 최신 상태 유지)
		Quest_details->SetText(FText::FromString(
			FString::Printf(TEXT("남은 시간 %.0f초  |  남은 적 %d"),
				CachedTimeRemaining, CachedEnemiesRemaining)));
	}
}



	// ------GameState 내용 바인딩.

void UCH3MainHUDWidget::HandleGamePlayStateChanged(EGamePlayState NewState)
{
	switch (NewState)
	{
	case EGamePlayState::GameClear:
		if (Quest) Quest->SetText(FText::FromString(TEXT("클리어!")));
		break;
	case EGamePlayState::GameOver:
		if (Quest) Quest->SetText(FText::FromString(TEXT("게임 오버")));
		break;
	case EGamePlayState::LevelUpPause:
		// [TODO] 레벨업 카드 UI 표시 (다음 단계)
		break;
	default:
		break;
	}
}

void UCH3MainHUDWidget::HandleScoreChanged(int32 NewScore)
{
	if (ScoreText) // Optional 위젯이라 null 체크 필수
	{
		ScoreText->SetText(FText::AsNumber(NewScore));
	}
}

void UCH3MainHUDWidget::HandleWaveChanged(int32 CurrentWave, int32 TotalWaves)
{
	if (Quest)
	{
		Quest->SetText(FText::FromString(
			FString::Printf(TEXT("Wave %d / %d 생존"), CurrentWave, TotalWaves)));
	}
}

void UCH3MainHUDWidget::HandleEnemiesRemainingChanged(int32 Remaining)
{
	CachedEnemiesRemaining = Remaining;
	RefreshQuestDetails();
}

void UCH3MainHUDWidget::HandleWaveTimeChanged(float TimeRemaining)
{
	CachedTimeRemaining = TimeRemaining;
	RefreshQuestDetails();
}




	//------ 캐릭터 <-> HUD 바인딩용 (캐릭터 코드 바인딩 전까지는 Test 함수로만 호출됨)

void UCH3MainHUDWidget::HandleHealthChanged(float CurrentHealth, float MaxHealth)
{
		// 현재는 PollPlayerStatus()가 매 프레임 이미 반영 중이라 아무도 호출하지 않음.
		// OnHealthChanged 델리게이트를 추가해 BindDelegates에서 연결하면 그때부터 쓰임.
	if (HealthBar && MaxHealth > 0.f)
	{
			// 체력은 보간 없이 즉시 반영 → "확 눈에 띄는" 느낌
		HealthBar->SetPercent(CurrentHealth / MaxHealth);
	}
}

void UCH3MainHUDWidget::HandleStaminaChanged(float CurrentStamina, float MaxStamina)
{
		//체력과 마찬가지 이유로 현재는 미사용.
	if (StaminaBar && MaxStamina > 0.f)
	{
		// 목표값만 갱신. 실제 화면 반영은 NativeTick의 FInterpTo가 부드럽게 처리
		StaminaBar->SetPercent(CurrentStamina / MaxStamina);
	}
}



	//탄약 갯수 관련.
void UCH3MainHUDWidget::HandleAmmoChanged(int32 CurrentAmmo, int32 MaxAmmo)
{
	if (AmmoText)
	{
		AmmoText->SetText(FText::FromString(
			FString::Printf(TEXT("%d / %d"), CurrentAmmo, MaxAmmo)));
	}
}
 

	//탄환 변경 시 UI 관련 부분. 스테이터스로 할지, 보이게 할지는 나중에.
void UCH3MainHUDWidget::HandleBulletSpeciesChanged(int32 WeaponIndex)
{
	// [TODO] 탄환 종류별 아이콘 표시용 UImage*가 아직 .h에 없음.
	// 예: UPROPERTY(meta=(BindWidgetOptional)) UImage* BulletTypeIcon;
	// 추가 후 WeaponIndex → 텍스처 매핑(TArray<UTexture2D*>)으로
	// BulletTypeIcon->SetBrushFromTexture(...) 호출하면 됨.
	// 지금은 링커 에러 방지용 빈 구현.
}
 



	//--------전투 관련 : 히트마커, 적 딜량 등등.
	//히트 마커 관련 부분. 크리티컬 포함. 근데 크리 안 넣을 예정 아닌가.
void UCH3MainHUDWidget::HandleHitConfirmed(bool bIsCritical)
{
	if (!HitMarkerImage) return;
 
	HitMarkerImage->SetColorAndOpacity(bIsCritical ? HitMarkerCriticalColor : HitMarkerNormalColor);
	HitMarkerImage->SetVisibility(ESlateVisibility::Visible);
 
	GetWorld()->GetTimerManager().ClearTimer(HitMarkerTimerHandle);
	GetWorld()->GetTimerManager().SetTimer(
		HitMarkerTimerHandle, this, &UCH3MainHUDWidget::HideHitMarker, HitMarkerDuration, false);
}


		//적이 대미지 입을 시 그 자리에 숫자 스폰.
void UCH3MainHUDWidget::HandleDamageDealt(float DamageAmount, FVector HitLocation, bool bIsCritical)
{
	if (!DamageNumberWidgetClass) return;
 
	FVector2D ScreenPos;
	APlayerController* PC = GetOwningPlayer();
	if (!PC) return;
 
	if (!UGameplayStatics::ProjectWorldToScreen(PC, HitLocation, ScreenPos))
	{
		return;
	}
 
	UCH3DamageNumberWidget* NewNumber = CreateWidget<UCH3DamageNumberWidget>(this, DamageNumberWidgetClass);
	if (!NewNumber) return;
 
	NewNumber->AddToViewport();
 
	const float DPIScale = UWidgetLayoutLibrary::GetViewportScale(this);
	NewNumber->SetPositionInViewport(ScreenPos / DPIScale, false);
 
	NewNumber->InitDamageNumber(DamageAmount, bIsCritical);
}



	// 킬 숫자
void UCH3MainHUDWidget::HandleEnemyKilled(EEnemyType EnemyType)
{
	if (!KillFeedEntryWidgetClass || !KillFeedBox) return;
	
		//몹 이름.
	FString EnemyName;

	
	/* ------몹 종류 -- 이 부분이 없을 시 "처치!" 문구만 나옴.
	 * GameplayTypes.h에 있던 내용이라는데, 이거 쓸 일이 있나?? 팀원한테 물어볼 것.
	 * => 사용 예정.
	 */
	switch (EnemyType)
	{
	case EEnemyType::Normal: EnemyName = TEXT("기본형 좀비"); break;
	case EEnemyType::Rush:   EnemyName = TEXT("돌진형 좀비");   break;
	case EEnemyType::Tanker: EnemyName = TEXT("탱커형 좀비");   break;
	case EEnemyType::Boss:   EnemyName = TEXT("보스");     break;
	default:                 EnemyName = TEXT("적");       break;
	}
	
 
	UCH3KillFeedEntryWidget* NewEntry = CreateWidget<UCH3KillFeedEntryWidget>(this, KillFeedEntryWidgetClass);
	if (NewEntry)
	{
		KillFeedBox->AddChildToVerticalBox(NewEntry);
		NewEntry->InitEntry(FText::FromString(EnemyName + TEXT(" 처치!")));
	}
}

	//히트마커 자동 숨김(몇초 뒤 히트마커 사라짐)
void UCH3MainHUDWidget::HideHitMarker()
{
	if (HitMarkerImage)
	{
		HitMarkerImage->SetVisibility(ESlateVisibility::Collapsed);
	}
}


//--------- 테스트 (발신원 없는 항목만 시뮬레이션)
 
void UCH3MainHUDWidget::Test_SimulateCombatFeedback()
{
	// 체력/스태미나는 PollPlayerStatus()가 실시간으로 이미 반영 중이라 테스트 제외.
	TestAmmo = (TestAmmo <= 0) ? 30 : TestAmmo - 3;
	HandleAmmoChanged(TestAmmo, 30);
 
	HandleHitConfirmed(FMath::RandBool());
 
	if (APawn* PlayerPawn = GetOwningPlayerPawn())
	{
		const FVector SpawnLoc = PlayerPawn->GetActorLocation() + PlayerPawn->GetActorForwardVector() * 200.f;
		HandleDamageDealt(FMath::RandRange(5, 45), SpawnLoc, FMath::RandBool());
	}
 
	HandleEnemyKilled(static_cast<EEnemyType>(FMath::RandRange(0, 3)));
}




	//-------------월드맵  미니맵좌표용.
FVector2D UCH3MainHUDWidget::WorldToMinimapPosition(const FVector& WorldLocation)
{
	APawn* PlayerPawn = GetOwningPlayerPawn();
	if (!PlayerPawn) return MinimapImagePixelSize * 0.5f;

	const FVector Rel = WorldLocation - PlayerPawn->GetActorLocation();
	// 플레이어 = 이미지 중앙(0.5, 0.5). 상대거리 / 촬영폭 만큼 중앙에서 이동.
	const float NormX = 0.5f + Rel.X / MinimapWorldSize.X;
	const float NormY = 0.5f + Rel.Y / MinimapWorldSize.Y;
	return FVector2D(NormX * MinimapImagePixelSize.X, NormY * MinimapImagePixelSize.Y);
}


	//----------크로스헤어 부분(달릴 때 벌어지게)
void UCH3MainHUDWidget::UpdateCrosshair(float DeltaTime)
{
		// 크로스헤어의 작대기 4개 중 하나라도 없으면(WBP에 미배치) 전체 스킵 - null 안전장치
	if (!CrosshairTop || !CrosshairBottom || !CrosshairLeft || !CrosshairRight)
	{
		return;
	}

		// 1) 현재 상태로 목표 벌어짐 결정
		// 정조준(ADS)은 아직 팀원 코드가 없으므로, 그 자리는 주석으로 남겨둠.
		// 지금은 "달리는 중"만 속도로 직접 판정 가능.
	float TargetSpread = SpreadIdle;

	
	if (APawn* Pawn = GetOwningPlayerPawn())
	{
			// 수평 속도만 봄(점프 등 수직 속도는 달림과 무관).
			// Velocity는 cm/s 단위. XY 평면 크기로 이동 속도 판정.
		const float PlanarSpeed = Pawn->GetVelocity().Size2D();
		if (PlanarSpeed > RunSpeedThreshold)
		{
			TargetSpread = SpreadRunning;
		}
	}
	

	// [TODO - 팀원 ADS 델리게이트/Getter 붙으면] => 정조준을 넣을 때.
	// if (bIsAiming) TargetSpread = SpreadADS;
	// 정조준이 달림보다 우선순위가 높아야 하면 이 분기를 위 달림 판정 뒤에 둘 것.

		// 2) 현재값을 목표값으로 부드럽게 보간(즉시 튀지 않고 스르륵)
	CurrentSpread = FMath::FInterpTo(CurrentSpread, TargetSpread, DeltaTime, SpreadInterpSpeed);

		// 3) 각 선분을 중심에서 CurrentSpread만큼 밀어냄(시작점은 가운데 모여있는 것.).
		// Alignment가 (0.5,0.5)라 Position은 "중심 기준 오프셋"으로 동작.
	if (UCanvasPanelSlot* S = Cast<UCanvasPanelSlot>(CrosshairTop->Slot))
		S->SetPosition(FVector2D(0.f, -CurrentSpread));
	if (UCanvasPanelSlot* S = Cast<UCanvasPanelSlot>(CrosshairBottom->Slot))
		S->SetPosition(FVector2D(0.f, CurrentSpread));
	if (UCanvasPanelSlot* S = Cast<UCanvasPanelSlot>(CrosshairLeft->Slot))
		S->SetPosition(FVector2D(-CurrentSpread, 0.f));
	if (UCanvasPanelSlot* S = Cast<UCanvasPanelSlot>(CrosshairRight->Slot))
		S->SetPosition(FVector2D(CurrentSpread, 0.f));
}