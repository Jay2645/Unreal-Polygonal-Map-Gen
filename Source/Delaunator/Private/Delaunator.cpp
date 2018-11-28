// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "Delaunator.h"

#define LOCTEXT_NAMESPACE "FDelaunatorModule"

DEFINE_LOG_CATEGORY(LogDelaunator);

void FDelaunatorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FDelaunatorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FDelaunatorModule, Delaunator)