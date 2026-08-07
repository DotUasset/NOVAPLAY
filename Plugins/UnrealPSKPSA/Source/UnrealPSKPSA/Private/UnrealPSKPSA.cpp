// Copyright Epic Games, Inc. All Rights Reserved.

#include "UnrealPSKPSA.h"

#include "PskFactory.h"
#include "PskxFactory.h"
#include "EditorReimportHandler.h"

#define LOCTEXT_NAMESPACE "FUnrealPSKPSAModule"

DEFINE_LOG_CATEGORY(LogUnrealPSKPSA);

void FUnrealPSKPSAModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	if (GEditor)
	{
		FReimportManager::Instance()->RegisterHandler(*GetMutableDefault<UPskFactory>());
		FReimportManager::Instance()->RegisterHandler(*GetMutableDefault<UPskxFactory>());
	}
}

void FUnrealPSKPSAModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	if (GEditor)
	{
		FReimportManager::Instance()->UnregisterHandler(*GetMutableDefault<UPskFactory>());
		FReimportManager::Instance()->UnregisterHandler(*GetMutableDefault<UPskxFactory>());
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FUnrealPSKPSAModule, UnrealPSKPSA)
