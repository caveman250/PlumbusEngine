#pragma once
#include "GameObject.h"
#include "TranslationComponent.h"
#include "mono_impl/mono_fwd.h"

MONO_EXPORT(mono_vec3) GetTranslation(plumbus::GameObject** obj)
{
	return (*obj)->GetComponent<plumbus::components::TranslationComponent>()->GetTranslation();
}

MONO_EXPORT(mono_vec3) GetRotation(plumbus::GameObject** obj)
{
	return (*obj)->GetComponent<plumbus::components::TranslationComponent>()->GetRotation();
}

MONO_EXPORT(mono_vec3) GetScale(plumbus::GameObject** obj)
{
	return (*obj)->GetComponent<plumbus::components::TranslationComponent>()->GetScale();
}

MONO_EXPORT(void) SetTranslation(plumbus::GameObject** obj, mono_vec3 translation)
{
	(*obj)->GetComponent<plumbus::components::TranslationComponent>()->SetTranslation({ translation.x, translation.y, translation.z });
}

MONO_EXPORT(void) SetRotation(plumbus::GameObject** obj, mono_vec3 rotation)
{
	(*obj)->GetComponent<plumbus::components::TranslationComponent>()->SetRotation({ rotation.x, rotation.y, rotation.z });
}

MONO_EXPORT(void) SetScale(plumbus::GameObject** obj, mono_vec3 scale)
{
	(*obj)->GetComponent<plumbus::components::TranslationComponent>()->SetScale({ scale.x, scale.y, scale.z });
}

MONO_EXPORT(void) Translate(plumbus::GameObject** obj, mono_vec3 translation)
{
	(*obj)->GetComponent<plumbus::components::TranslationComponent>()->Translate({ translation.x, translation.y, translation.z });
}

MONO_EXPORT(void) Rotate(plumbus::GameObject** obj, mono_vec3 rotation)
{
	(*obj)->GetComponent<plumbus::components::TranslationComponent>()->Rotate({ rotation.x, rotation.y, rotation.z });
}

MONO_EXPORT(void) Scale(plumbus::GameObject** obj, mono_vec3 scale)
{
	(*obj)->GetComponent<plumbus::components::TranslationComponent>()->Scale({ scale.x, scale.y, scale.z });
}