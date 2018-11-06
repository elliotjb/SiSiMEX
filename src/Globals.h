#pragma once
#include <cinttypes>

// Constants ///////////////////////////////////////////////////////////

/** Hostname (or IP address) of the YellowPages process. */
static const char *HOSTNAME_YP = "localhost";

/** Listen port used by the YellowPages process. */
static const uint16_t LISTEN_PORT_YP = 8000;

/** Listen port used by the multi-agent application. */
static const uint16_t LISTEN_PORT_AGENTS = 8001;

/**
 * Constant used to specify that a message was sent to,
 * or received from no agent. This is the case when
 * communicating with the YellowPages. Yellow pages is
 * a global service that contains information about
 * contributor agents, but uses no agents to work.
 */
static const uint16_t NULL_AGENT_ID = 0;

/**
 * Constant used to specify a null item ID.
 * This is used mainly in MCCs without an item
 * constraint, that is, those contributors that don't
 * require another item in exchange to provide its
 * resource. In those cases, the constraint item
 * attribute is assigned the NULL_ITEM_ID value.
 */
static const uint16_t NULL_ITEM_ID = 9999;
