#pragma once

#include "ckafprod_config.h"

#include <inttypes.h>

// TODO Kevin: Only needed for internal API, create a separate header file, where we can rely on curl.h
#ifdef USE_VM
#include <curl/curl.h>
#endif

#include "sim/simulation.h"

#define VM_PROTOCOL_MAXLEN 8
#define VM_URL_MAXLEN 256
#define VM_BUFFER_SIZE      10 * 1024 * 1024
typedef struct {

#ifdef USE_VM
    CURL * curl;
    struct curl_slist * headers;
#else
    #define CURLcode void
#endif

    char url[VM_URL_MAXLEN];
    char protocol[VM_PROTOCOL_MAXLEN];  // TODO Kevin: Can/Should we make 'protocol' part of the URL?

    char buffer[VM_BUFFER_SIZE];
    size_t buffer_usage;

    int push_retries;

    // vm_init_args_t * args;
} vm_connection_t;

/**
 * @brief Arguments to use while initializing a connection to Victoria Metrics
 * 
 * Arguments are currently const,
 * so we are sure that they can be shared between threads.
 */
typedef const struct {
    int use_ssl;
    uint16_t port;
    int skip_verify;
    int verbose;
    char * username;
    char * password;
    char * server_ip;
} vm_init_args_t;

CURLcode vm_init(vm_init_args_t * args, vm_connection_t * vm_connection_out);

/**
 * @brief Add a new line to the VM buffer
 * 
 * Automatically calls vm_push() before the buffer overflows.
 * 
 * @param metric_line string metric-line to send to VM
 * @param vm_connection Connection potentially used to push to VM
 */
CURLcode vm_add(char * metric_line, vm_connection_t * vm_connection);
CURLcode vm_add_usage_line(usage_line_t * usage_line, vm_connection_t * vm_connection);


/**
 * @brief Manually transmit the current buffer to VM.
 * 
 * @param vm_connection 
 * @return CURLcode Indicates whether the transmission was successful, after accounting for retries.
 */
CURLcode vm_push(vm_connection_t * vm_connection);

/**
 * @brief Should be called after vm_init() when closing the connection VM.
 * 
 * Frees memory that was dynamically allocated by vm_init()
 * 
 * @param vm_connection Connection to close
 */
void vm_cleanup(vm_connection_t * vm_connection);