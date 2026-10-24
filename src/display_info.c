/* Supply the already-advertised HDR capability at the dictionary levels used
 * by newer macOS clients. Preserve the native PIN, renderer, and media paths.
 * Original function address is checked by build_mac_display_compat.py.
 */
#include <dlfcn.h>
#include <stdint.h>
#include <stddef.h>
#include <android/log.h>
typedef const void *CFRef;
extern uintptr_t CFGetTypeID(CFRef);
extern uintptr_t CFDictionaryGetTypeID(void);
extern uintptr_t CFArrayGetTypeID(void);
extern uintptr_t CFStringGetTypeID(void);
extern CFRef CFStringCreateWithCString(CFRef, const char *, uint32_t);
extern CFRef CFDictionaryGetValue(CFRef, CFRef);
extern void CFDictionarySetValue(void *, CFRef, CFRef);
extern intptr_t CFArrayGetCount(CFRef);
extern CFRef CFArrayGetValueAtIndex(CFRef, intptr_t);
extern void CFRelease(CFRef);

static int is_type(CFRef object, uintptr_t type) {
    return object && CFGetTypeID(object) == type;
}

__attribute__((visibility("default")))
void *AirPlayCopyServerInfo(void *server, void *a1, void *a2, int *error) {
    Dl_info image;
    if (!dladdr((void *)CFDictionaryGetValue, &image) || !image.dli_fbase) {
        if (error) *error = -6700;
        return NULL;
    }
    typedef void *(*Original)(void *, void *, void *, int *);
    Original original = (Original)((uintptr_t)image.dli_fbase + 0x16ec0d);
    void *info = original(server, a1, a2, error);
    if (!is_type(info, CFDictionaryGetTypeID())) return info;
    CFRef displays_key = CFStringCreateWithCString(NULL, "displays", 0x08000100);
    CFRef hdr_key = CFStringCreateWithCString(NULL, "HDRInfo", 0x08000100);
    CFRef capability_key = CFStringCreateWithCString(NULL, "receiverHDRCapability", 0x08000100);
    if (!displays_key || !hdr_key || !capability_key) goto done;
    CFRef displays = CFDictionaryGetValue(info, displays_key);
    if (!is_type(displays, CFArrayGetTypeID())) goto done;
    intptr_t count = CFArrayGetCount(displays);
    for (intptr_t i = 0; i < count; ++i) {
        CFRef display = CFArrayGetValueAtIndex(displays, i);
        if (!is_type(display, CFDictionaryGetTypeID())) continue;
        CFRef hdr = CFDictionaryGetValue(display, hdr_key);
        if (!is_type(hdr, CFDictionaryGetTypeID())) continue;
        CFRef capability = CFDictionaryGetValue(hdr, capability_key);
        if (!is_type(capability, CFStringGetTypeID())) continue;
        if (!CFDictionaryGetValue(display, capability_key))
            CFDictionarySetValue((void *)display, capability_key, capability);
        if (!CFDictionaryGetValue(info, capability_key))
            CFDictionarySetValue(info, capability_key, capability);
        __android_log_print(ANDROID_LOG_INFO, "AirPlayDisplayCompat",
                           "Propagated existing HDR capability for display %ld", (long)i);
    }
done:
    if (displays_key) CFRelease(displays_key);
    if (hdr_key) CFRelease(hdr_key);
    if (capability_key) CFRelease(capability_key);
    return info;
}
