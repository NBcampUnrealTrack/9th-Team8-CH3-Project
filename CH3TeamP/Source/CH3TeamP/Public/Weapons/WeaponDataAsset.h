#pragma once
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "WeaponDataAsset.generated.h"

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	RipleGun UMETA(DisplayName = "Riple"),
};

UCLASS()
class CH3TEAMP_API UWeaponDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon")
	EWeaponType WeaponType = EWeaponType::RipleGun;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon")
	USkeletalMesh* Mesh = nullptr;

	// 무기 메시가 손에 붙을 때 적용할 보정 위치/회전/스케일
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon")
	FTransform AttachTransform = FTransform::Identity;
	
	UPROPERTY(EditAnywhere)
	float FireInterval = 0.1f;
	

	
};