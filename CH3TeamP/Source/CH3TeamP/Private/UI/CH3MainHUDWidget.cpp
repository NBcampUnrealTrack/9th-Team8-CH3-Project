// 메인위젯.

             

//위젯 넣는 아이콘(?)들.
#include "UI/CH3MainHUDWidget.h"
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
#include "UI/MiniMap/MiniMapWidget.h"
#include "UI/MiniMap/CH3MinimapCaptureActor.h"
#include "UI/DamageWidget/CH3DamageNumberWidget.h"
#include "UI/DamageWidget/CH3KillFeedEntryWidget.h"
#include "CH3GameState.h"   // GameState 바인딩을 위해 include.
#include "CH3TeamProjectGameMode.h"  //강화 카드 인벤토리를 위해. 
#include "UI/Upgrade/CH3UpgradeListWidget.h"

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
					// 앵커를 캔버스 정중앙(0.5,0.5)에 고정.
				CanvasSlot->SetAnchors(FAnchors(0.5f, 0.5f));

					// 마커의 중심을 앵커에 맞춤. 없으면 마커 왼쪽위 모서리가 기준이라 치우침.
				CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));

					// 위치는 0이 곧 미니맵 정중앙.
				CanvasSlot->SetPosition(FVector2D(0.f, 0.f));
				
					// 슬롯 크기를 이미지(24×24)와 일치시킴. 크기를 안 잡으면 슬롯이 이미지를 늘려서 마커가 가로로 퍼짐.
				CanvasSlot->SetSize(FVector2D(24.f, 24.f));
			}
		}
	}
	
	
	
		//2) 히트마커 이미지
	if (HitMarkerImage)
	{
		HitMarkerImage->SetVisibility(ESlateVisibility::Collapsed);
	}
	
	
		//3) 탄종 아이콘 초기값. 시작은 항상 일반탄.
	if (BulletTypeIcon && NormalAmmoTexture)
	{
		BulletTypeIcon->SetBrushFromTexture(NormalAmmoTexture, false);
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
	
	PollPlayerStatus(DeltaTime);
	
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
		// 누적 처치 수.
	GS->OnKillCountChanged.AddDynamic(this, &UCH3MainHUDWidget::HandleKillCountChanged);
	
		// 카드 강화 부분.
		// 강화 확정 구독 — 획득 목록 갱신용.
		//   ACH3HUD도 같은 델리게이트를 구독하지만(카드 닫기용), 멀티캐스트라 둘 다 받아도 됨.
	if (ACH3TeamProjectGameMode* GM = GetWorld()->GetAuthGameMode<ACH3TeamProjectGameMode>())
	{
		GM->OnUpgradeConfirmed.AddDynamic(this, &UCH3MainHUDWidget::HandleUpgradeAcquired);
		
			// [추가] 적 처치 델리게이트 부분. 방송 구독 — 킬 피드 한 줄 표시용.
		GM->OnEnemyKilledNotify.AddDynamic(this, &UCH3MainHUDWidget::HandleEnemyKilled);
	}
	

	
	// [중요] 초기값 동기화:
	// 델리게이트는 "값이 바뀌는 순간"만 알려줌. HUD가 생성되기 전에 이미 바뀐 값들은 알림을 못 받았음.
	// 그래서 Getter로 현재값을 한 번 직접 읽어와서 화면을 맞춰준다.
	HandleGamePlayStateChanged(GS->GetPlayState());
	HandleScoreChanged(GS->GetTotalScore());
	HandleWaveChanged(GS->GetCurrentWave(), GS->GetTotalWaves());
	HandleEnemiesRemainingChanged(GS->GetEnemiesRemaining());
	HandleWaveTimeChanged(GS->GetWaveTimeRemaining());
		// 킬 카운트 초기값 동기화. HUD 생성 전에 이미 잡은 적이 있을 수 있음.
	HandleKillCountChanged(GS->GetTotalKillCount());
	

		// [해제] 캐릭터의 EXP/레벨업 델리게이트는 이미 존재 확인됨. 바인딩.
		// HealthChanged/StaminaChanged는 캐릭터 쪽에 델리게이트 자체가 없어 여전히 폴링(PollPlayerStatus) 사용.
	if (APlayerCharacter* PC = Cast<APlayerCharacter>(GetOwningPlayerPawn()))
	{
		PC->OnEXPChanged.AddDynamic(this, &UCH3MainHUDWidget::HandleEXPChanged);
		PC->OnLevelUp.AddDynamic(this, &UCH3MainHUDWidget::HandleLevelUp);

		HandleEXPChanged(PC->CurrentEXP, PC->MaxEXP);
		HandleLevelUp(PC->CurrentLevel);
	}
	
	
	// [TODO - 캐릭터 담당자 코드 완성 후 봉인해제]
	// 캐릭터에 OnHealthChanged / OnStaminaChanged 델리게이트가 생기면 여기서 바인딩:
	// if (APlayerCharacter* PC = Cast<APlayerCharacter>(GetOwningPlayerPawn()))
	// {
	//     PC->OnHealthChanged.AddDynamic(this, &UCH3MainHUDWidget::HandleHealthChanged);
	//     PC->OnStaminaChanged.AddDynamic(this, &UCH3MainHUDWidget::HandleStaminaChanged);
	// }
	
	
	
	// ※ 체력/스태미나는 PollPlayerStatus()가 매 프레임 담당. 아래 HandleHealthChanged/
	//   HandleStaminaChanged는 팀원이 나중에 델리게이트를 추가할 때를 위한 "대기 중" 구현.
	// ※ 경험치/레벨업은 캐릭터에 델리게이트가 이미 있어 위에서 바로 바인딩함(대기 아님).
 
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


	//체력/ 스태미나/ 총알을 를 매 프레임 읽어오기.
void UCH3MainHUDWidget::PollPlayerStatus(float DeltaTime)
{
	if (!IsValid(CachedPlayer)) //cachedPlayer가 아닌 isvalid인 이유 : 
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
		
		// DeltaTime을 함께 넘김 (맥동 계산에 필요)
		UpdateLowHealthEffect(CachedPlayer->HealthComp->GetHealthPercent(), DeltaTime);
	}
 
	if (StaminaBar && CachedPlayer->StaminaComp)
	{
		StaminaBar->SetPercent(CachedPlayer->StaminaComp->GetStaminaPercent());
	}
	
	
		// 맵 경계 이탈 경고.
	UpdateBoundaryWarning(CachedPlayer->GetActorLocation());
	
	
	
		// 탄약 폴링.
		// CurrentAmmoCount / MaxAmmo 둘 다 PlayerCharacter의 public 멤버라 그냥 읽으면 됨.
		// 값이 직전과 같으면 아무것도 안 함 → 발사/장전 순간에만 텍스트 갱신됨.
	const int32 NowAmmo = CachedPlayer->CurrentAmmoCount;
	const int32 NowMaxAmmo = CachedPlayer->MaxAmmo;

	if (NowAmmo != LastPolledAmmo || NowMaxAmmo != LastPolledMaxAmmo)
	{
		LastPolledAmmo = NowAmmo;       // 캐시 갱신
		LastPolledMaxAmmo = NowMaxAmmo;

			// 이미 만들어둔 함수를 그대로 재사용. (델리게이트가 생기면 이 함수만 그대로 바인딩.)
		HandleAmmoChanged(NowAmmo, NowMaxAmmo);
	}
	
	
	
}

		//---------퀘스트 관련(묶음)
void UCH3MainHUDWidget::RefreshQuestDetails()
{
	if (Quest_Timedetails)
	{
		// 남은시간과 남은적은 서로 다른 델리게이트로 따로 도착함.
		// 캐시해둔 두 값을 합치지 않음.
		// 각각 한 줄씩 표시. (이벤트 도착 순서와 무관하게 항상 최신 상태 유지)
		Quest_Timedetails->SetText(FText::FromString(
			FString::Printf(TEXT("남은 시간 %.0f초"),
				CachedTimeRemaining)));
	}
	
	if (Quest_Mobdetails)
	{
		Quest_Mobdetails->SetText(FText::FromString(
			FString::Printf(TEXT("남은 적 %d"),
				CachedEnemiesRemaining)));
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

	// 킬 카운트.
void UCH3MainHUDWidget::HandleKillCountChanged(int32 NewKillCount)
{
	if (KillCountText)
	{
		KillCountText->SetText(FText::FromString(
			FString::Printf(TEXT("처치 : %d"), NewKillCount)));
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


	//경험치바 부분.
void UCH3MainHUDWidget::HandleEXPChanged(float NewCurrentEXP, float NewMaxEXP)
{
	if (EXPBar && NewMaxEXP > 0.f)
	{
		EXPBar->SetPercent(NewCurrentEXP / NewMaxEXP);
	}
}
	// 레벨업 부분.
void UCH3MainHUDWidget::HandleLevelUp(int32 NewLevel)
{
	if (LevelText)
	{
		LevelText->SetText(FText::FromString(FString::Printf(TEXT("Lv.%d"), NewLevel)));
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
	
		// 정조준 중엔 스코프로 화면 전체를 덮고, 크로스헤어는 숨김.
		// 스코프와 크로스헤어가 동시에 보이면 어색하므로 서로 배타적으로 처리.
	const bool bAiming = IsValid(CachedPlayer) && CachedPlayer->bIsAiming;
	
	
	if (ScopeOverlay)
	{
		ScopeOverlay->SetVisibility(bAiming ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}

	if (CrosshairTop) CrosshairTop->SetVisibility(bAiming ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	if (CrosshairBottom) CrosshairBottom->SetVisibility(bAiming ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	if (CrosshairLeft) CrosshairLeft->SetVisibility(bAiming ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	if (CrosshairRight) CrosshairRight->SetVisibility(bAiming ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);

	if (!CrosshairTop || !CrosshairBottom || !CrosshairLeft || !CrosshairRight)
	{
		return;
	}
	
		// 크로스헤어의 작대기 4개 중 하나라도 없으면(WBP에 미배치) 전체 스킵 - null 안전장치
	if (!CrosshairTop || !CrosshairBottom || !CrosshairLeft || !CrosshairRight)
	{
		return;
	}

	
	/*기존 코드 - PC없이 OwningPlayer를 받기 위해서 썼던 부분.
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
	*/
	
		//cachedPlayer가 OwningPlayer를 이곳에서 따로 받지 않아도 되도록 함.
		//APlayerCharacter* CachedPlayer; 변수를 공유하고 있어서, pollplayer에서 운용됨.
		// 1) 현재 상태로 목표 벌어짐 결정
	float TargetSpread = SpreadIdle;

			// [변경] GetOwningPlayerPawn() → PollPlayerStatus()가 이미 확보해둔 CachedPlayer 재사용.
			// 같은 프레임에 이미 캐스팅이 끝나 있으므로 Pawn을 다시 가져올 이유가 없음.
	if (IsValid(CachedPlayer))
	{
			// 수평 속도만 봄(점프 등 수직 속도는 이동과 무관). Velocity는 cm/s 단위.
		const float PlanarSpeed = CachedPlayer->GetVelocity().Size2D();

		if (CachedPlayer->bIsAiming)
		{
				// [변경] TODO 해제. 팀원 코드에 bIsAiming(public)이 있는 것을 확인함.
				// 정조준이 최우선 — 정조준 중엔 이동 여부와 무관하게 가장 좁게.
			TargetSpread = SpreadADS;
		}
		else if (PlanarSpeed > WalkSpeedThreshold)
		{
				// [변경] 달림 판정을 속도 임계값(RunSpeedThreshold)이 아니라, 이미 설정된 bIsSprinting 플래그로 교체.
				// 이유 : 굳이 달리는 속도로 할 필요가 없을듯.
				// 만약 물 등에서 속도 변하면 그때는 이용될지도 모름.
				// 플래그를 쓰면 나중에 속도 수치가 바뀌어도 UI가 안 깨진다.
				// WalkSpeedThreshold는 "실제로 움직이고 있나"만 보는 용도 —
				// 제자리에서 Shift만 눌러도 Idle이 유지되도록.
			TargetSpread = CachedPlayer->bIsSprinting ? SpreadRunning : SpreadWalking;
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



	// 체력 저하 시 화면 이펙트 부분.
void UCH3MainHUDWidget::UpdateLowHealthEffect(float HealthPercent, float DeltaTime)
{
		// WBP에 없으면(BindWidgetOptional이라 null 가능) 조용히 종료.
	if (!LowHealthVignette)
	{
		return;
	}

		// 1) 20% 초과면 완전히 끄고 끝
		// Collapsed = 렌더링 자체를 생략(성능). 붉은 기운이 전혀 없어야 하는 구간.
	if (HealthPercent > LowHealthThreshold)
	{
		LowHealthVignette->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

		// 체력 20% 이하. 여기 도달 시 이펙트가 보이게 키기.
		// HitTestInvisible = 화면엔 보이되 마우스 클릭은 통과(아래 버튼 안 막음).
	LowHealthVignette->SetVisibility(ESlateVisibility::HitTestInvisible);

		// 2) 강도 계산: 20% -> 0.0, 10% -> 1.0 으로 매핑
		// GetMappedRangeValueClamped: 입력범위를 출력범위로 비례 변환 + 범위 밖은 잘라줌.
		// 입력을 (0.20, 0.10) 순서로 준 건, 체력이 낮을수록 강도가 커지게 방향을 뒤집은 것.
		// (정의 위치: Engine/Source/Runtime/Core/Public/Math/UnrealMathUtility.h)
	const float Intensity = FMath::GetMappedRangeValueClamped(
		FVector2D(LowHealthThreshold, CriticalHealthThreshold),
		FVector2D(0.0f, 1.0f),
		HealthPercent);

		// 3) 심장박동 맥동
		// 시간을 계속 쌓아 sin에 넣으면 -1~1 파도가 됨. 그걸 0~1로 바꿔 밝기 진동으로 씀.
		// PulseAccumulator가 멤버여야 하는 이유: 지역 변수면 매 프레임 0으로 초기화돼 파도가 안 생김.
	PulseAccumulator += DeltaTime * PulseSpeed;
	const float Pulse = (FMath::Sin(PulseAccumulator) * 0.5f + 0.5f);	// 0~1 진동

		// 기본 강도 70% + 맥동 30%. 항상 어느 정도 붉되, 그 위에서 두근거림.
	const float FinalAlpha = Intensity * (0.7f + 0.3f * Pulse);

		// 4) 투명도 적용
		// 색은 흰색(1,1,1)으로 두고 알파만 조절. 텍스처가 이미 붉으니 색을 곱하지 않음.
	LowHealthVignette->SetColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, FinalAlpha));

		// 5) 시야 축소: 강도가 셀수록 이미지를 확대
		// 붉은 가장자리가 안쪽으로 밀려들어와 시야가 좁아 보임. 카메라 안 건드리고 UI만으로 구현.
		// RenderScale은 렌더링만 키움(레이아웃 영향 없음). 1.0배 → 최대 1.6배.
	const float Scale = 1.0f + Intensity * 0.2f;
	LowHealthVignette->SetRenderScale(FVector2D(Scale, Scale));
}

void UCH3MainHUDWidget::UpdateBoundaryWarning(const FVector& PlayerLocation)
{
	if (!BoundaryWarningVignette)
	{
		return;
	}

	// FVector 성분은 UE5 LWC 때문에 double. float 멤버와 섞이면 Max/Min 템플릿이 타입을 못 정함.
	const float PlayerX = static_cast<float>(PlayerLocation.X);
	const float PlayerY = static_cast<float>(PlayerLocation.Y);

	// 네 벽까지의 거리. 안쪽이면 양수, 넘으면 음수.
	// [변경] 기존 Max3(0.f, ...) 방식은 "얼마나 벗어났나"만 남기고 "얼마나 안쪽인가"를
	//        전부 0으로 뭉갰음. 플레이어가 벽에 막혀 못 나가는 이상 그 값은 항상 0이라
	//        경고 강도를 만들 수 없었다. 그래서 부호 있는 거리로 교체.
	const float DistMinX = PlayerX - BoundaryMinX;
	const float DistMaxX = BoundaryMaxX - PlayerX;
	const float DistMinY = PlayerY - BoundaryMinY;
	const float DistMaxY = BoundaryMaxY - PlayerY;

	// 가장 가까운 벽까지의 거리. 이 값 하나로 강도를 정한다.
	const float NearestDist = FMath::Min(
		FMath::Min(DistMinX, DistMaxX),
		FMath::Min(DistMinY, DistMaxY));

	// 마진보다 안쪽이면 경고 없음. Collapsed로 렌더링 자체를 생략.
	if (NearestDist >= BoundaryWarningMargin)
	{
		BoundaryWarningVignette->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	BoundaryWarningVignette->SetVisibility(ESlateVisibility::HitTestInvisible);

	// 마진 지점에서 0.0, 벽에서 1.0.
	// 입력을 (Margin, 0) 순서로 준 건 "가까울수록 진해지게" 방향을 뒤집은 것.
	// (저체력에서 체력이 낮을수록 진해지게 한 것과 같은 패턴)
	const float Intensity = FMath::GetMappedRangeValueClamped(
		FVector2D(BoundaryWarningMargin, 0.f),
		FVector2D(0.f, 1.f),
		NearestDist);

	BoundaryWarningVignette->SetColorAndOpacity(
		FLinearColor(0.f, 0.f, 0.f, Intensity * BoundaryWarningMaxAlpha));
}








		// 카드 강화의 인벤토리 부분.
void UCH3MainHUDWidget::HandleUpgradeAcquired(EUpgradeType ChosenUpgrade, AController* ForPlayer)
{
		
		// 관통탄/폭발탄은 "중첩 능력치"가 아니라 "현재 장착 상태".
		// 인벤토리엔 안 넣고, 여기서 HUD 아이콘만 교체함.
		// (캐릭터의 CurrentAmmoType이 protected라 못 읽지만, 카드 선택 신호로 대신 판단)
	if (ChosenUpgrade == EUpgradeType::PiercingAmmo)
	{
		if (BulletTypeIcon && PiercingAmmoTexture)
		{
			BulletTypeIcon->SetBrushFromTexture(PiercingAmmoTexture, false);
		}
		return;
	}
	if (ChosenUpgrade == EUpgradeType::ExplosiveAmmo)
	{
		if (BulletTypeIcon && ExplosiveAmmoTexture)
		{
			BulletTypeIcon->SetBrushFromTexture(ExplosiveAmmoTexture, false);
		}
		return;
	}
	

	// 해당 종류 개수 +1. FindOrAdd: 없으면 0으로 만들고 반환, 있으면 기존 값 반환.
	int32& Count = AcquiredUpgrades.FindOrAdd(ChosenUpgrade);
	Count++;

			/* 이 부분은 버프처럼 화면에 상시 표시할 때 사용하는 거.
	// 목록 위젯에 갱신된 맵 전달.
	if (UpgradeListWidget)
	{
		UpgradeListWidget->RefreshList(AcquiredUpgrades);
	}
	*/
	
		// 강화 카드 리스트가 창이 열려 있을 때만 다시 그리도록.
		//  닫혀 있으면 개수만 올려두고, I키로 열 때 ToggleInventory가 갱신함.
		//  안 보이는 창을 그리는 건 낭비라서 IsInViewport()로 확인.
	if (UpgradeListWidget && UpgradeListWidget->IsInViewport())
	{
		UpgradeListWidget->RefreshList(AcquiredUpgrades);
	}
}



	// 카드 강화 인벤토리 용 토글 함수들.
void UCH3MainHUDWidget::ToggleUpgradeInventory()
{
	// [디버그] I키 입력이 여기까지 도달하는지 확인용. 확인 후 삭제.
	UE_LOG(LogTemp, Warning, TEXT("[HUD] ToggleUpgradeInventory 호출됨"));
	
	if (!UpgradeListWidgetClass)
	{
		return;
	}
	if (!UpgradeListWidget)
	{
		UpgradeListWidget = CreateWidget<UCH3UpgradeListWidget>(GetOwningPlayer(), UpgradeListWidgetClass);
	}
	if (UpgradeListWidget)
	{
		UpgradeListWidget->ToggleInventory(AcquiredUpgrades);
	}
}

