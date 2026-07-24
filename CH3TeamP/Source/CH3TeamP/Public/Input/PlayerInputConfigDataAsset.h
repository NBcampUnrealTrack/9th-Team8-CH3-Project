#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PlayerInputConfigDataAsset.generated.h"

UCLASS(BlueprintType)
class CH3TEAMP_API UPlayerInputConfigDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	// Input Mapping Context
	UPROPERTY(EditAnywhere, Category="Input|Context")
	class UInputMappingContext* InputMappingContext;

	// Input Actions
	UPROPERTY(EditAnywhere, Category="Input|Actions")
	class UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, Category="Input|Actions")
	class UInputAction* LookAction;

	UPROPERTY(EditAnywhere, Category="Input|Actions")
	class UInputAction* JumpAction;
	
    UPROPERTY(EditAnywhere, Category="Input|Actions")
	class UInputAction* CrouchAction;

	UPROPERTY(EditAnywhere, Category="Input|Actions")
	class UInputAction* SprintAction;
	
	UPROPERTY(EditAnywhere, Category="Input|Actions")
	class UInputAction* AimAction;

	UPROPERTY(EditAnywhere, Category="Input|Actions")
	class UInputAction* RipleAction;
	
	UPROPERTY(EditAnywhere, Category="Input|Actions")
	class UInputAction* FireAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input|Actions")
	UInputAction* ReloadAction;
	
};	