# 第三方声明

本项目不是全量开源的 Sony AirPlay 实现。完整 APK 中原厂与其他第三方组件仍受各自权利和许可约束；此文件不是完整 APK 的许可证合规认证。

## UxPlay 协议响应表

- 来源：https://github.com/FDH2/UxPlay/blob/master/lib/fairplay_playfair.c
- 文件：`src/fp_messages.inc`
- 原版权声明：Copyright (C) 2018 Juho Vähä-Herttua
- 许可证：GNU Lesser General Public License version 2.1 or later（LGPL-2.1-or-later）。
- 本项目改动：提取响应表和头部，接入当前扩展的 HTTP 协商处理；保留原版权与许可证声明。

本仓库提供的扩展源码 `src/fp_static.c` 与 `src/display_info.c` 的本项目新增部分以 LGPL-2.1-or-later 提供；涉及的原厂接口、二进制及其他权利不因此改换许可证。扩展源码与构建方式随仓库提供，许可全文见 `LICENSES/LGPL-2.1.txt`。

## 原厂与其他组件

Release APK 基于 Sony 海外固件中的 AirPlay 应用，并包含 Sony、Apple 及其他供应商/开源组件相关代码与资源。本项目不拥有或重新授予这些部分的权利，不得把扩展的 LGPL 声明解释为整个 APK 采用 LGPL。

未给仓库设置覆盖所有文件与 Release APK 的统一开源许可证。用户须分别核对原厂软件条款和相关组件许可。
