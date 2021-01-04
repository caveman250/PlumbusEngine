#pragma once
#include "GameComponent.h"
#include "GameObject.h"
#include "TranslationComponent.h"
#include "mono_impl/mono_fwd.h"

enum ComponentType
{
	TranslationComponent,
};

MONO_EXPORT(void) GetComponent(int type);