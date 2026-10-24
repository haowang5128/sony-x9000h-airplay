# Sony X9000H 国行 AirPlay 兼容移植

源码与文档公开提供；最终 APK 位于本仓库 Releases。

将 Sony 海外固件中的原生 AirPlay 接收器适配到国行 X9000H。本项目是非官方个人兼容实验，与 Sony、Apple 无隶属或授权关系。

**最终 APK 在 [Releases](https://github.com/haowang5128/sony-x9000h-airplay/releases) 下载，不放在 Git 源码仓库中。** 本版本针对下述设备实测，其他电视和固件不保证兼容。无需刷入海外系统，不需要 root。

## 已验证设备与固件

| 项目 | 实测记录 |
|---|---|
| 电视型号 | **Sony KD-75X9000H（中国大陆版）** |
| 系统平台 | BRAVIA_VH1_CN |
| Android | 10 |
| 国行系统构建号 | **623021** |
| 完整系统构建标识 | `BRAVIA_VH1_CN-user 10 QTG3.200305.006.S397 623021 release-keys` |
| 海外 APK 提取源 | 欧洲版 XH90/XH92 固件 **v6.2322**，文件 `sony_atvotafull_2020_1602322100_eua_auth.zip` |
| 原 AirPlay 应用版本 | 2.0.A.7.2（versionCode 2065000702） |
| 实测日期 | 2026-09-28 |

**623021 是实测国行电视构建号，v6.2322 是提取 APK 的海外固件版本，两者不要混淆。** 未保存电视设置页面的完整 PKG 版本字符串，因此不把推算的 PKG 字符串当作实测数据。

## 验证结果与限制

- Mac 系统“屏幕镜像”：画面和声音已由用户确认正常。
- 电视按 Home 后重新投屏：授予下述权限后可自动切到投屏画面。
- 电视重启后：用户确认无需手动打开 AirPlay，联网后可直接投屏。
- 手机画面、声音在前一软件认证版本中已验证；最终加入 Mac 显示器补丁后未单独重复手机回归测试。
- 没有主动配置 Pro 专属启动项；没有单独记录退出 Pro 模式后的状态，因此不声称已做普通模式的独立对照验证。
- **Mac Infuse 应用内投屏仍未修好**：NAS/SMB 片源曾向接收器传入 `infuse://host/stream.eac3`，电视播放器不识别该协议。可尝试 Mac 系统屏幕镜像后在 Infuse 全屏播放。
- HomeKit、多房间音频、受保护媒体、长期稳定性、待机网络唤醒未验证。
- 基于原生 AirPlay 2 接收器，不代表修改版已通过 Apple 认证或完整支持全部 AirPlay 2 功能；能力字段也不等于已验证 4K60/HDR。

## 安装

准备 Android Platform Tools（ADB），电视与电脑位于可互通的局域网。开启电视调试并在电视上接受连接授权。将下列 `TV_IP` 换成你的电视地址；调试端口以实际配置为准。

```sh
adb connect TV_IP:5555
adb -s TV_IP:5555 install --no-incremental -r Sony-X9000H-AirPlay-Mac-verified.apk
```

`-r` 保留应用数据和配对。若已有不同签名版本，可能出现签名冲突；不要直接卸载或清数据，应先确认原有版本及恢复方式。首次安装成功后执行：

```sh
adb -s TV_IP:5555 shell appops set com.sony.dtv.airplayapp SYSTEM_ALERT_WINDOW allow
adb -s TV_IP:5555 shell appops get com.sony.dtv.airplayapp SYSTEM_ALERT_WINDOW
adb -s TV_IP:5555 shell am start-foreground-service -n com.sony.dtv.airplayapp/com.sony.dtv.airplayserver.AirPlayService
adb -s TV_IP:5555 shell am start -n com.sony.dtv.airplayapp/.AirPlayPlayer
```

**“显示在其他应用上层”权限是 Home 后自动切换画面的必要安装步骤（本次 Android 10 实测）。** 前台服务不等于播放器 Activity 一直可见；未授权时可能连接成功但仍停留在电视首页。该授权是设备状态，不会随 APK 自动设置。撤回时将 `allow` 改为 `default`。

从 Mac 控制中心打开“屏幕镜像”，选择电视，输入电视当前显示的 PIN。确认画面和声音，然后测试 Home 后重连以及电视重启后的自动启动。

若只是安装空间不足，可自行清理可重建缓存后重试，不要清除 AirPlay 应用数据。完成安装后可按需要关闭调试，正常 AirPlay 使用不依赖 ADB。

## 电视端设置入口

原 APK 没有电视桌面的 `LEANBACK_LAUNCHER` 入口，国行设置/输入源菜单也未随本移植新增菜单，因此应用面板找不到图标不代表安装失败。

可手动打开保留的设置 Activity：

```sh
adb -s TV_IP:5555 shell am start -n com.sony.dtv.airplayapp/.AirPlaySettingEntryActivity
```

正常投屏不需要每次打开此页面。海外 Sony 官方一般从 INPUT → AirPlay → AirPlay & HomeKit settings 进入；本项目没有移植完整海外系统菜单。

## 实现概述

- 保留已有 native loader，解决系统库命名空间和 JNI 注册问题；适配国行区域 API 差异。
- 调整发现与配对能力，保留普通 HAP PIN 验证及原生加密路径。
- 构建时接入软件媒体协商扩展，避免此前失败的运行时代码页修改。
- 将接收器已有的 `receiverHDRCapability` 补到 Mac 读取的字典层级，解决本机 Mac 创建虚拟显示器时的 `CFEqual(NULL)` 崩溃。
- 未修改 Mac 系统二进制、电视 SELinux 策略或刷写系统分区。

`src/` 保存本次扩展的 C 源码与协议表，构建说明见 [BUILD.md](BUILD.md)。**本仓库不是整个 Sony 应用的开源源码，也不声称可以仅凭这些源码从零构建完整 APK。** 原厂二进制及早期 Java 移植包含另有权利归属的内容。

## 校验

Release APK：`Sony-X9000H-AirPlay-Mac-verified.apk`

```text
SHA256 f66a336af18f5ea7e80afb9e946c2b31fb4123d824551c2e4b47edeb0daedda2
```

该哈希对应已安装实测文件。仓库不包含签名私钥、电视配对凭据、设备日志或完整固件。

## 免责声明与权利说明

1. 本项目为非官方兼容实验，按现状提供，不承诺适用于其他设备、固件或后续系统更新。请自行评估安装、升级和恢复风险，保留原有配置。
2. APK 包含 Sony 原生软件及其他第三方组件，相关著作权、商标、专利和其他权利归原权利人所有。本项目未取得或声称取得 Sony/Apple 对完整修改版的再分发、商业使用或认证授权。
3. 补丁中来自 UxPlay 的内容遵循 LGPL-2.1-or-later；参见 [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md) 和随附许可证。该许可证不覆盖整个 Sony APK，也不赋予其再分发权。
4. “学习交流”“免费”或本免责声明均不构成授权，也不保证分发、修改或商业使用合法。使用者和分发者应核实适用法律、原厂许可与第三方许可证要求；请勿冒充官方或将本项目描述为官方授权产品。
5. 不对未验证功能作保证。免责表述仅在适用法律允许的范围内有效，不排除依法不能排除的责任。
6. 若权利人认为项目内容侵犯其权利，可通过仓库 Issue 联系维护者，说明具体内容及权利依据，以便核实处理。
