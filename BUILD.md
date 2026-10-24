# 扩展源码与构建范围

`src/fp_static.c`、`src/fp_messages.inc`、`src/display_info.c` 是最终 `libxh90fpauth.so` 使用的扩展源码。协议表保留原作者声明，参见 THIRD_PARTY_NOTICES.md。

本轮工具：Android NDK 27.2.12479018，armv7a-linux-androideabi29-clang。按本机 NDK 路径编译：

```sh
export NDK=/path/to/android-ndk-27.2.12479018
"$NDK/toolchains/llvm/prebuilt/darwin-x86_64/bin/armv7a-linux-androideabi29-clang" \
  -shared -fPIC -O2 -mthumb -Wall -Wextra -Werror=implicit-function-declaration \
  -Wl,-soname,libxh90fpauth.so src/fp_static.c src/display_info.c \
  -o libxh90fpauth.so -llog -ldl
```

这个库使用特定 Sony 2.0.A.7.2 二进制的结构布局和原函数偏移，不能通用于其他版本。生成 `.so` 不等于生成完整 APK。

修改该扩展时，可用自己编译的库替换 Release APK 中的 `lib/armeabi-v7a/libxh90fpauth.so`，保留其他文件，删除失效的旧 APK 签名元数据后重新 zipalign 并签名。库必须 ZIP_STORED，因为 APK 使用 extractNativeLibs=false。签名后运行 apksigner verify。自有签名不能覆盖维护者签名的现有安装；请在自己的测试环境处理安装和数据备份。

补丁源码允许依据随附 LGPL 条款修改、调试和替换；这不代表原厂组件获得额外授权。完整原厂应用、原始固件和早期修改版 DEX 不作为源码上传。发布包也不包含设备配对数据或签名私钥。
