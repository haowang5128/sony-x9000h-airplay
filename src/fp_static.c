#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <android/log.h>
#define logp(...) __android_log_print(ANDROID_LOG_INFO, "AirPlaySoftwareAuth", __VA_ARGS__)
/* Software screen-mirroring negotiation, restricted to native HAP sessions.
 * No MFi certificate, authentication flag, PIN check, or media key is altered.
 * Media remains handled by AirPlayReceiverSessionSetHomeKitSecurityContext.
 * ABI offsets are specific to the hash-checked Sony 2.0.A.7.2 binary.
 */
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>
#include "fp_messages.inc"

extern int HTTPMessageSetBody(void *, const char *, const void *, size_t);
#define fp_set_body HTTPMessageSetBody

static void *fp_ptr(void *object, size_t offset) {
    void *value;
    memcpy(&value, (char *)object + offset, sizeof(value));
    return value;
}

__attribute__((visibility("default"))) int MFiSAP_Exchange(void *connection, void *request) {
    if (!connection || !request) return 500;
    /* Same native HAP context used by the primary SETUP handler. */
    if (!fp_ptr(connection, 0xec)) {
        logp("FP compatibility: reject request without HAP security context");
        return 403;
    }
    void *http = fp_ptr(connection, 4);
    if (!http) return 500;
    void *response = fp_ptr(http, 0xac);
    if (!response) return 500;
    const unsigned char *body = fp_ptr(request, 0x10a4);
    uint32_t size;
    memcpy(&size, (char *)request + 0x10b0, sizeof(size));
    if (!body || (size != 16 && size != 164)) return 400;
    if (memcmp(body, "FPLY\x03\x01", 6) || body[7] != 0 ||
        body[8] != 0 || body[9] != 0 || body[10] != 0 ||
        body[11] != size - 12) return 400;
    unsigned char reply[142];
    size_t reply_size;
    if (size == 16 && body[6] == 1 && body[14] < 4) {
        memcpy(reply, reply_message[body[14]], 142);
        reply_size = 142;
    } else if (size == 164 && body[6] == 3) {
        memcpy(reply, fp_header, 12);
        memcpy(reply + 12, body + 144, 20);
        reply_size = 32;
    } else {
        return 400;
    }
    int err = fp_set_body(response, "application/octet-stream", reply, reply_size);
    logp("FP compatibility: HAP request %u bytes, response %zu bytes, result %d",
         size, reply_size, err);
    return err ? 500 : 200;
}

