// Original Work Copyright (c) 2010 Amit J Patel, Modified Work Copyright (c) 2016 Jay M Stevens

#pragma once

#include "Object.h"
#include "DummyObject.generated.h"

/**
 * This is a dummy class with no functionality whatsoever.
 * There is a bug in the Unreal Header Tool (UHT) which causes Plugins to not work
 * unless there is this dummy file created as the first thing in the project.
 */
UCLASS()
class UDummyObject : public UObject
{
	GENERATED_BODY()
};
