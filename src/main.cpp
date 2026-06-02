#include <jni.h>
#include <string.h>
#include <android/log.h>
#include "zygisk.hpp"

#define LOG_TAG "ZygiskBypass"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

const char* FAKE_JSON = "{\"success\": true, \"data\": {\"license_key\": \"LQMAX-639E-5547-640E-B17C\", \"key_id\": \"mafia-1295\", \"status\": \"active\", \"duration_days\": 0.0833, \"created_at\": \"2026-05-25 14:44:12\", \"expires_at\": null, \"first_login_at\": null, \"max_devices\": 1, \"product_code\": \"MAFIA\", \"product_name\": \"MAFIA\"}}";

int (*orig_SSL_read)(void* ssl, void* buf, int num);

int my_SSL_read(void* ssl, void* buf, int num) {
    int ret = orig_SSL_read(ssl, buf, num);
    if (ret > 0 && buf != nullptr) {
        char* str_buf = (char*)buf;
        if (strstr(str_buf, "HTTP/1.1 200") != nullptr && strstr(str_buf, "application/json") != nullptr) {
            char* body = strstr(str_buf, "\r\n\r\n");
            if (body != nullptr) {
                body += 4;
                if (strstr(body, "success") != nullptr || strstr(body, "error") != nullptr) {
                    LOGD("Intercepted API response!");
                    size_t fake_len = strlen(FAKE_JSON);
                    strcpy(body, FAKE_JSON);
                    ret = (body - str_buf) + fake_len;
                }
            }
        }
    }
    return ret;
}

class MyModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api *api, JNIEnv *env) override {
        LOGD("Zygisk Module Loaded!");
        // Dùng sức mạnh gốc của Zygisk, không cần Dobby!
        api->pltHookRegister(".*", "SSL_read", (void*)my_SSL_read, (void**)&orig_SSL_read);
        api->pltHookCommit();
    }
};

REGISTER_ZYGISK_MODULE(MyModule)
