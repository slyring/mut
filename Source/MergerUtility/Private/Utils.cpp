#include "Utils.h"
#include "MergerUtility/Public/Utils.h"

#include "ContentBrowserModule.h"
#include "Editor.h"
#include "IContentBrowserSingleton.h"
#include "IMeshMergeUtilities.h"
#include "MeshMergeModule.h"
#include "ScopedTransaction.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/StaticMeshActor.h"
#include "Misc/MessageDialog.h"
#include "Misc/ScopedSlowTask.h"

#define LOCTEXT_NAMESPACE "MergerUtils"

#if WITH_EDITOR
bool UUtils::MergeActorsWithSettings(const FString& PackageName, const TArray<AActor*>& SelectedActors,
                                     const FMeshMergingSettings& Settings,
                                     bool bReplaceSourceActors, AStaticMeshActor*& MeshActor)

{
	const IMeshMergeUtilities& MeshUtilities = FModuleManager::Get().LoadModuleChecked<
		IMeshMergeModule>("MeshMergeUtilities").GetUtilities();
	TArray<AActor*> Actors;

	FVector MergedActorLocation;
	TArray<UObject*> AssetsToSync;
	TArray<ULevel*> UniqueLevels;

	// Extracting static mesh components from the selected mesh components in the dialog
	TArray<UPrimitiveComponent*> ComponentsToMerge;

	BuildActorsListFromMergeComponentsData(ComponentsToMerge, SelectedActors, &UniqueLevels);

	// This restriction is only for replacement of selected actors with merged mesh actor
	if (UniqueLevels.Num() > 1 && bReplaceSourceActors)
	{
		FText Message = NSLOCTEXT("UnrealEd", "FailedToMergeActorsSublevels_Msg",
		                          "The selected actors should be in the same level");
		const FText Title = NSLOCTEXT("UnrealEd", "FailedToMergeActors_Title", "Unable to merge actors");
		FMessageDialog::Open(EAppMsgType::Ok, Message, &Title);
		return false;
	}

	// Merge...
	{
		FScopedSlowTask SlowTask(0, LOCTEXT("MergingActorsSlowTask", "Merging actors..."));
		SlowTask.MakeDialog();


		if (ComponentsToMerge.Num())
		{
			UWorld* World = ComponentsToMerge[0]->GetWorld();
			checkf(World != nullptr, TEXT("Invalid World retrieved from Mesh components"));
			const float ScreenAreaSize = TNumericLimits<float>::Max();

			// If the merge destination package already exists, it is possible that the mesh is already used in a scene somewhere, or its materials or even just its textures.
			// Static primitives uniform buffers could become invalid after the operation completes and lead to memory corruption. To avoid it, we force a global reregister.
			if (FindObject<UObject>(nullptr, *PackageName))
			{
				FGlobalComponentReregisterContext GlobalReregister;
				MeshUtilities.MergeComponentsToStaticMesh(ComponentsToMerge, World, Settings, nullptr,
				                                          nullptr, PackageName, AssetsToSync, MergedActorLocation,
				                                          ScreenAreaSize, true);
			}
			else
			{
				MeshUtilities.MergeComponentsToStaticMesh(ComponentsToMerge, World, Settings, nullptr,
				                                          nullptr, PackageName, AssetsToSync, MergedActorLocation,
				                                          ScreenAreaSize, true);
			}
		}
	}

	if (AssetsToSync.Num())
	{
		FAssetRegistryModule& AssetRegistry = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(
			"AssetRegistry");
		int32 AssetCount = AssetsToSync.Num();
		for (int32 AssetIndex = 0; AssetIndex < AssetCount; AssetIndex++)
		{
			AssetRegistry.AssetCreated(AssetsToSync[AssetIndex]);
			GEditor->BroadcastObjectReimported(AssetsToSync[AssetIndex]);
		}

		//Also notify the content browser that the new assets exists
		FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>(
			"ContentBrowser");
		ContentBrowserModule.Get().SyncBrowserToAssets(AssetsToSync, true);

		// Place new mesh in the world

		UStaticMesh* MergedMesh = nullptr;
		if (AssetsToSync.FindItemByClass(&MergedMesh))
		{
			const FScopedTransaction Transaction(LOCTEXT("PlaceMergedActor", "Place Merged Actor"));
			UniqueLevels[0]->Modify();

			UWorld* World = UniqueLevels[0]->OwningWorld;
			FActorSpawnParameters Params;
			Params.OverrideLevel = UniqueLevels[0];
			FRotator MergedActorRotation(ForceInit);

			AStaticMeshActor* MergedActor = World->SpawnActor<AStaticMeshActor>(
				MergedActorLocation, MergedActorRotation, Params);
			MergedActor->GetStaticMeshComponent()->SetStaticMesh(MergedMesh);
			MergedActor->SetActorLabel(MergedMesh->GetName());
			World->UpdateCullDistanceVolumes(MergedActor, MergedActor->GetStaticMeshComponent());
			GEditor->SelectNone(true, true);
			GEditor->SelectActor(MergedActor, true, true);
			MeshActor = MergedActor;

			if (bReplaceSourceActors)
			{
				// Remove source actors
				for (AActor* Actor : Actors)
				{
					Actor->Destroy();
				}
			}
		}
	}
	return true;
}

void UUtils::BuildActorsListFromMergeComponentsData(TArray<UPrimitiveComponent*>& OutComponentsData,
                                                    const TArray<AActor*>& Actors, TArray<ULevel*>* OutLevels)
{
	for (AActor* Actor : Actors)
	{
		check(Actor != nullptr);

		TArray<UPrimitiveComponent*> PrimComponents;
		Actor->GetComponents(PrimComponents);
		for (UPrimitiveComponent* PrimComponent : PrimComponents)
			OutComponentsData.Add(PrimComponent);

		if (OutLevels)
			OutLevels->AddUnique(Actor->GetLevel());
	}
}
#endif
