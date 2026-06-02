#include <jni.h>
#include <string.h>
#include <stdio.h>
#include <android/log.h>
#include "zygisk.hpp"

#define LOG_TAG "ZygiskBypass"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

int (*orig_SSL_read)(void* ssl, void* buf, int num);

int my_SSL_read(void* ssl, void* buf, int num) {
    int ret = orig_SSL_read(ssl, buf, num);
    if (ret > 0 && buf != nullptr) {
        char* str_buf = (char*)buf;
        
        // Nếu đây là gói tin HTTP và là API JSON
        if (strstr(str_buf, "HTTP/1.1 ") != nullptr && strstr(str_buf, "application/json") != nullptr) {
            // Chặn bắt bất kỳ gói tin nào có chứa chữ success hoặc message (dấu hiệu của API check Key)
            if (strstr(str_buf, "\"success\"") != nullptr || strstr(str_buf, "\"message\"") != nullptr) {
                
                LOGD("Intercepted API response!");
                
                // Chế tạo Key VIP Vĩnh viễn (9999 ngày)
                const char* fake_json = "{\"success\":true,\"data\":{\"license_key\":\"VIP\",\"key_id\":\"1\",\"status\":\"active\",\"duration_days\":9999,\"created_at\":\"2024-01-01 00:00:00\",\"expires_at\":null,\"first_login_at\":null,\"max_devices\":999,\"product_code\":\"MAFIA\",\"product_name\":\"MAFIA\"}}";
                
                // Đóng gói lại thành Chuẩn HTTP với Content-Length chính xác tuyệt đối
                char fake_resp[1024];
                sprintf(fake_resp, 
                    "HTTP/1.1 200 OK\r\n"
                    "Content-Type: application/json\r\n"
                    "Content-Length: %zu\r\n"
                    "Connection: close\r\n"
                    "\r\n"
                    "%s", strlen(fake_json), fake_json);
                
                size_t fake_len = strlen(fake_resp);
                
                // Đè toàn bộ gói tin fake lên gói tin thật
                if (num >= fake_len) {
                    memcpy(buf, fake_resp, fake_len);
                    // Báo cho Game biết độ dài mới của Gói tin
                    ret = fake_len;
                    LOGD("Spoofed FULL HTTP payload injected. You have lifetime VIP!");
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
        api->pltHookRegister(0, 0, "SSL_read", (void*)my_SSL_read, (void**)&orig_SSL_read);
        api->pltHookCommit();
    }
};

REGISTER_ZYGISK_MODULE(MyModule)
