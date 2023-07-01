#include "kvs_event_manager.h"

void KVSEventManager::notify(const String &p_event, const Variant &p_data, const Variant::Type p_data_type) {
	for (KVSComponent* component : _components) {
		component->update(p_event, p_data, p_data_type);
	}
}

void KVSEventManager::notify(const String &p_event) {
	for (KVSComponent* component : _components) {
		component->update(p_event);
	}
}

void KVSEventManager::subscribe(KVSComponent *component) {
	if (component == nullptr) {
		ERR_FAIL_MSG("Cannot subscribe a null component");
	}
	_components.push_back(component);
}

void KVSEventManager::unsubscribe(KVSComponent *component) {
	if (component == nullptr) {
		ERR_FAIL_MSG("Cannot unsubscribe a null component");
	}
	if (!_components.erase(component)) {
		ERR_FAIL_MSG("Cannot unsubscribe a component that is not subscribed");
	}
}
