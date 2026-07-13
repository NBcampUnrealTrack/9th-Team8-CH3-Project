#pragma once

#include "CoreMinimal.h"
#include "CombatTypes.generated.h"

UENUM(BlueprintType)
enum class EAmmoType : uint8
{
	Normal		UMETA(DisplayName = "Normal"), // 기본
	Piercing	UMETA(DisplayName = "Piercing"), // 관통
	Explosive	UMETA(DisplayName = "Explosive") // 폭발
};
