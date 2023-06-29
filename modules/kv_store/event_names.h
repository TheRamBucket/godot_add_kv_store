
#ifndef EVENT_NAMES_H
#define EVENT_NAMES_H

#include "core/variant/variant.h"

namespace KVSEvents {

// ----------------------------------------------------------------
// Global Events
// ----------------------------------------------------------------
	const String INIT_COMPONENT = "init_component";
	const String UPDATE_COMPONENT = "update_component";
	const String SHUTDOWN_COMPONENT = "shutdown_component";
	const String COMPONENT_READY = "component_ready";
	const String COMPONENT_ERROR = "component_error";
	const String COMPONENT_SHUTDOWN = "component_shutdown";
	const String COMPONENT_UPDATED = "component_updated";
	const String COMPONENT_INITIALIZED = "component_initialized";
	const String GET_STATE = "get_state";
	const String STATE_CHANGED = "state_changed";
	const String ERROR = "error";


// ----------------------------------------------------------------
// Red Black Tree Events
// ----------------------------------------------------------------
	const String KEY_ADDED = "key_added";
	const String KEY_REMOVED = "key_removed";
	const String RBT_FULL = "rbt_full";


// ----------------------------------------------------------------
// Key Value Store Events
// ----------------------------------------------------------------
	const String ADD_KEY = "add_key";
	const String REMOVE_KEY = "remove_key";


}

#endif
