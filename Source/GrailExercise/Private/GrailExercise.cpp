// Copyright Epic Games, Inc. All Rights Reserved.

#include "GrailExercise.h"

#include "SGrailDictionaryWidget.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "ToolMenus.h"

#define LOCTEXT_NAMESPACE "FGrailExerciseModule"

void FGrailExerciseModule::StartupModule()
{
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(TabId,
	FOnSpawnTab::CreateRaw(this, &FGrailExerciseModule::OnSpawnPluginTab))
	.SetDisplayName(FText::FromString(TEXT("Grail Dictionary Tool")))
	.SetMenuType(ETabSpawnerMenuType::Enabled);
}

void FGrailExerciseModule::ShutdownModule()
{
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(TabId);
}

TSharedRef<SDockTab> FGrailExerciseModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SGrailDictionaryWidget)
		];
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FGrailExerciseModule, GrailExercise)