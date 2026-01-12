#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ARKitCurveMappingAsset.generated.h"

USTRUCT(BlueprintType)
struct FARKitTargetCurves
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mapping")
	TArray<FName> TargetCurves;
};

UCLASS(BlueprintType)
class ARKITCURVEMAPPER_API UARKitCurveMappingAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	// Key = ARKit blendshape name (e.g. "jawOpen")
	// Value = list of avatar curve names to drive (e.g. ["Jaw_Open", "Jaw_Open_Extra"])
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Mapping")
	TMap<FName, FARKitTargetCurves> Map;
};
