#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Utils.generated.h"

/**
 * 
 */
UCLASS()
class UUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	UFUNCTION(BlueprintCallable, Category="Merger Utility")
	static bool MergeActorsWithSettings(const FString& PackageName,
	                                    const TArray<AActor*>& SelectedActors, const FMeshMergingSettings& Settings,
	                                    bool bReplaceSourceActors, class AStaticMeshActor*& MeshActor);

	static void BuildActorsListFromMergeComponentsData(TArray<UPrimitiveComponent*>& OutComponentsData,
	                                                   const TArray<AActor*>& Actors, TArray<ULevel*>* OutLevels);

	UFUNCTION(BlueprintCallable, Category="Merger Utility")
	static TArray<FString> SortActorGroup(const TArray<FString>& Keys);

#endif
};
