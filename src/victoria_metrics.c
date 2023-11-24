
#include "victoria_metrics.h"

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <sys/utsname.h>

#include "sim/simulation.h"

size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
    return size * nmemb;
}

CURLcode vm_init(vm_init_args_t * args, vm_connection_t * vm_connection_out) {

    /* Sadly, curl_easy_init() returns a heap-allocated object.
        So we must remember to free it later. */
    vm_connection_out->curl = curl_easy_init();

    CURLcode res;

    /* Shorthand names */
    CURL * curl = vm_connection_out->curl;
    char * url = vm_connection_out->url;
    char * protocol = vm_connection_out->protocol;
    struct curl_slist * headers = vm_connection_out->headers;

    if (curl == NULL) {
        // vm_running = 0;
        printf("curl_easy_init() failed\n");
        return CURLE_FAILED_INIT;
    }

    int vm_running = 1;

    static struct utsname info;
	uname(&info);
    const char * hostname = info.nodename;

    snprintf(vm_connection_out->protocol, VM_PROTOCOL_MAXLEN, "http");

    if (args->use_ssl) {
        char * ssl = "s";
        strncat(vm_connection_out->protocol, ssl, VM_PROTOCOL_MAXLEN);
    }

    if (args->skip_verify) {
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    }

    if (args->verbose) {
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    } else {
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    }

    if (args->username && args->password) {
        curl_easy_setopt(curl, CURLOPT_USERNAME, args->username);
        curl_easy_setopt(curl, CURLOPT_PASSWORD, args->password);
        curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
    }

    // Test connection
    snprintf(url, VM_URL_MAXLEN, "%s://%s:%d/prometheus/api/v1/query", protocol, args->server_ip, args->port);
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "query=test42");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, 12);
    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        printf("Failed test of connection: %s\n", curl_easy_strerror(res));
        vm_running = 0;
    }
    long response_code;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    if (response_code != 200 && res == CURLE_OK) {
        printf("Failed test with response code: %ld\n", response_code);
        vm_running = 0;
    }

    // Resume building of header for push
    snprintf(url, VM_URL_MAXLEN, "%s://%s:%d/api/v1/import/prometheus?extra_label=instance=%s", protocol, args->server_ip, args->port, hostname);
    curl_easy_setopt(curl, CURLOPT_URL, url);
    headers = curl_slist_append(headers, "Content-Type: text/plain");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    if (args->verbose) {
        printf("Full URL: %s\n", url);
    }

    if (vm_running) {
        printf("Connection established to %s://%s:%d\n", protocol, args->server_ip, args->port);
    }

    return res;
}

CURLcode vm_push(vm_connection_t * vm_connection) {

    // // Lock the buffer mutex
    // pthread_mutex_lock(&buffer_mutex);
    if (vm_connection->buffer_usage == 0) {
        // pthread_mutex_unlock(&buffer_mutex);
        return CURLE_GOT_NOTHING;
    }

    CURL * curl = vm_connection->curl;

    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, vm_connection->buffer_usage);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, vm_connection->buffer);

    CURLcode res = curl_easy_perform(curl);  // Always try at least once, also tells us whether we should retry.
    for (int tries = vm_connection->push_retries; tries > 0; tries--) {
        res = curl_easy_perform(curl);
    }
    
    if (res != CURLE_OK) {
        printf("Failed push: %s", curl_easy_strerror(res));
    }

    vm_connection->buffer_usage = 0;
    // // Unlock the buffer mutex
    // pthread_mutex_unlock(&buffer_mutex);

    return res;
}

void vm_cleanup(vm_connection_t * vm_connection) {
    printf("vm push stopped\n");
    // Clean up
    if (vm_connection->curl) {
        curl_easy_cleanup(vm_connection->curl);
        vm_connection->curl = NULL;
    }
    if (vm_connection->headers) {
        curl_slist_free_all(vm_connection->headers);
        vm_connection->headers = NULL;
    }
#if 0
    curl_global_cleanup();  // Must only be called once, so we call it in main.c
    if (args->username) {
        free(args->username);
        args->username = NULL;
    }
    if (args->password) {
        free(args->password);
        args->password = NULL;
    }
    if (args->server_ip) {
        free(args->server_ip);
        args->server_ip = NULL;
    }
    free(args);
#endif
}

/* TODO Kevin: Ideally, we should be able to differentiate
    whether vm_push() wasn't called, or finished successfully.  */
CURLcode vm_add(char * metric_line, vm_connection_t * vm_connection) {

    // // Lock the buffer mutex
    // pthread_mutex_lock(&buffer_mutex);

    #define buffer_usage vm_connection->buffer_usage

    // Check if there's enough space in the buffer
    size_t line_len = strlen(metric_line);
    CURLcode res = CURLE_OK;
    if (buffer_usage + line_len >= VM_BUFFER_SIZE) {
        res = vm_push(vm_connection);
    }

    // Add the new metric line to the buffer
    strcpy(vm_connection->buffer + buffer_usage, metric_line);
    buffer_usage += line_len;

    // // Unlock the buffer mutex
    // pthread_mutex_unlock(&buffer_mutex);
    return res;
}

CURLcode vm_add_usage_line(usage_line_t * usage_line, vm_connection_t * vm_connection) {
    
    char outstr[256] = {};

    #define STRINGIFY(x) #x

    char * power = STRINGIFY(POWER);
    char * water = STRINGIFY(WATER);
    char * heat = STRINGIFY(HEAT);

    snprintf(outstr, 256,   "%s{} %f %"PRIu64"\n"
                            "%s{} %f %"PRIu64"\n"
                            "%s{} %f %"PRIu64"\n",  power, usage_line->power_usage, usage_line->unix_timestamp_seconds,
                                                    water, usage_line->water_usage, usage_line->unix_timestamp_seconds,
                                                    heat, usage_line->heat_usage, usage_line->unix_timestamp_seconds);

    return vm_add(outstr, vm_connection);
}

/* TODO Kevin: Our equivalent to vm_add_param() should account for the 
    fact that we may want different metrics to have different destinations. */  
#if 0
void vm_add_param(param_t * param) {

    if(param->type == PARAM_TYPE_STRING || param->type == PARAM_TYPE_DATA){
        return;
    }
    static char outstr[1000] = {};
    static char valstr[100] = {};
    int arr_cnt = param->array_size;
    if (arr_cnt < 0)
        arr_cnt = 1;

    struct timeval tv;
	gettimeofday(&tv, NULL);
	uint64_t time_ms = ((uint64_t) tv.tv_sec * 1000000 + tv.tv_usec) / 1000;

    for (int j = 0; j < arr_cnt; j++) {
        param_value_str(param, j, valstr, 100);
        snprintf(outstr, 1000, "%s{node=\"%u\", idx=\"%u\"} %s %"PRIu64"\n", param->name, param->node, j, valstr, time_ms);
        vm_add(outstr);
    }
}
#endif
