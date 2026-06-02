#include <jni.h>
#include <string.h>
#include <android/log.h>
#include <stdio.h>
#include <unistd.h>
#include <dlfcn.h>
#include "dobby.h"

#define LOG_TAG "ZygiskBypass"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

const char* FAKE_JSON = "{\"success\": true, \"data\": {\"license_key\": \"LQMAX-639E-5547-640E-B17C\", \"key_id\": \"mafia-1295\", \"status\": \"active\", \"duration_days\": 0.0833, \"created_at\": \"2026-05-25 14:44:12\", \"expires_at\": null, \"first_login_at\": null, \"max_devices\": 1, \"product_code\": \"MAFIA\", \"product_name\": \"MAFIA\"}}";

// Hook variables
void* (*orig_SSL_read)(void* ssl, void* buf, int num);

// Hook function
void* my_SSL_read(void* ssl, void* buf, int num) {
    // Call original SSL_read
    int ret = (int)orig_SSL_read(ssl, buf, num);
    
    // Check if there is data
    if (ret > 0 && buf != nullptr) {
        char* str_buf = (char*)buf;
        
        // If the buffer contains our target API signature
        // The game sends POST to api.php?action=public_check
        // But SSL_read will read the RESPONSE from the server
        // So we look for "success": false or similar response patterns from that server
        // Or we can just blindly spoof if we know the length and it looks like JSON
        
        // The easiest way is to intercept curl_easy_perform or libcurl but those are static
        // Intercepting SSL_read is generic. We check if the response looks like from ngocbaocheat
        
        if (strstr(str_buf, "HTTP/1.1 200") != nullptr && strstr(str_buf, "application/json") != nullptr) {
            // Find the body
            char* body = strstr(str_buf, "\r\n\r\n");
            if (body != nullptr) {
                body += 4;
                
                // If it's the license check response
                if (strstr(body, "success") != nullptr || strstr(body, "error") != nullptr) {
                    LOGD("Intercepted API response!");
                    
                    // Overwrite the body with our fake JSON
                    size_t fake_len = strlen(FAKE_JSON);
                    strcpy(body, FAKE_JSON);
                    
                    // Adjust return value (length)
                    // We need to fix the Content-Length header ideally, but usually it's fine
                    // if we just return the new total size
                    ret = (body - str_buf) + fake_len;
                    
                    LOGD("Spoofed JSON payload injected.");
                }
            }
        }
    }
    return (void*)ret;
}

// Zygisk companion
static void companion_handler(int i) {
    // Companion logic if needed
}

// Module entry
extern "C" {
    void zygisk_module_entry(void* api_ptr, JNIEnv* env) {
        LOGD("Zygisk Module Loaded!");
        
        // Find libssl.so
        void* handle = dlopen("libssl.so", RTLD_LAZY);
        if (handle) {
            void* ssl_read_ptr = dlsym(handle, "SSL_read");
            if (ssl_read_ptr) {
                LOGD("Found SSL_read at %p", ssl_read_ptr);
                DobbyHook(ssl_read_ptr, (void*)my_SSL_read, (void**)&orig_SSL_read);
                LOGD("Hooked SSL_read!");
            } else {
                LOGE("Could not find SSL_read symbol");
            }
            dlclose(handle);
        } else {
            LOGE("Could not open libssl.so");
        }
    }
}
