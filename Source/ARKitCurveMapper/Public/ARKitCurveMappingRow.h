#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ARKitCurveMappingRow.generated.h"

USTRUCT(BlueprintType)
struct FARKitMappingRow : public FTableRowBase
{
	GENERATED_BODY()

	// ARKit key, e.g. jawOpen
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ARKit;

	// Targets separated by '|', e.g. Jaw_Open|Jaw_Open_Extra
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Targets;
};
