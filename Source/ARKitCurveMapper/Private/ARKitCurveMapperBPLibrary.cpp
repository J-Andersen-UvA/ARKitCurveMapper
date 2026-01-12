#include "ARKitCurveMapperBPLibrary.h"
#include "Engine/DataTable.h"
#include "ARKitCurveMappingRow.h"

static float ClampScaled(const float V, const float Scale, const float MinV, const float MaxV)
{
	return FMath::Clamp(V * Scale, MinV, MaxV);
}

void UARKitCurveMapperBPLibrary::BuildCurveMapFromARKitDataTable(
	const TArray<FName>& PropertyNames,
	const TArray<float>& PropertyValues,
	const UDataTable* MappingTable,
	TMap<FName, float>& OutCurveMap,
	const bool bAccumulateIfAlreadySet,
	const float GlobalScale,
	const float ClampMin,
	const float ClampMax)
{
	OutCurveMap.Reset();
	if (!MappingTable) return;

	const int32 Count = FMath::Min(PropertyNames.Num(), PropertyValues.Num());
	if (Count <= 0) return;

	for (int32 i = 0; i < Count; ++i)
	{
		const FName ArkitKey = PropertyNames[i];
		const float V = ClampScaled(PropertyValues[i], GlobalScale, ClampMin, ClampMax);

		// RowName == ARKit key (because CSV Name column == ARKit)
		const FARKitMappingRow* Row = MappingTable->FindRow<FARKitMappingRow>(ArkitKey, TEXT("ARKitCurveMapper"));
		if (!Row) continue;

		TArray<FString> Targets;
		Row->Targets.ParseIntoArray(Targets, TEXT("|"), true);

		for (FString& T : Targets)
		{
			T.TrimStartAndEndInline();
			if (T.IsEmpty()) continue;

			const FName TargetName(*T);

			if (bAccumulateIfAlreadySet)
			{
				float& Existing = OutCurveMap.FindOrAdd(TargetName);
				Existing += V;
			}
			else
			{
				OutCurveMap.Add(TargetName, V);
			}
		}
	}
}
