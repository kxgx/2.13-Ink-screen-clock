/**
 * @file    api.h
 * @brief   HTTP API server — exposes all display elements as JSON
 *
 * Starts a minimal HTTP server on port 8080 in a background thread.
 * GET / returns current screen data as JSON.
 */

#ifndef API_H
#define API_H

/** Start the HTTP API server.  Runs in its own thread, never returns. */
void api_server_start(void);

#endif /* API_H */
