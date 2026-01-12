#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ARKitCurveMapperBPLibrary.generated.h"

UCLASS()
class ARKITCURVEMAPPER_API UARKitCurveMapperBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "ARKit|CurveMapper")
	static void BuildCurveMapFromARKitDataTable(
		const TArray<FName>& PropertyNames,
		const TArray<float>& PropertyValues,
		const UDataTable* MappingTable,
		UPARAM(ref) TMap<FName, float>& OutCurveMap,
		bool bAccumulateIfAlreadySet = false,
		float GlobalScale = 1.0f,
		float ClampMin = 0.0f,
		float ClampMax = 1.0f
	);
};
