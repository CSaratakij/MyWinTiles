# MyWinTiles
![pic](screenshots/pic.png)
- Tiling window manager for Windows 10
- Built on top of explorer shell
- Inspired by i3 window manager

# Dependencies
- Visual Studio 2017
- Visual C++
- Windows 10 SDK
- AutoHotkey

# Getting Start
1. Disable Windows global hotkey and restart your computer
```
HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Policies\Explorer
NoWinKeys REG_DWORD 0x00000001 (1)
```
2. Compile & Run

# Keyboard shortcut
- Take a look at [Default keyboard shortcut](Keybind.md)

# Additional
- You might want to use [MyWinBar](https://github.com/CSaratakij/MyWinBar), an AppBar for MyWinTiles.
- You might want to use [Wox](https://github.com/Wox-launcher/Wox), an alternative App Launcher of choice for MyWinTiles.
Use with [Optional Disable Key](optinal_disable_keys.ahk) for convenience.

# Issue
- Test with Windows 10 (Other windows version don't officially support.)
- If you want to control some window that start with admin permission, Run this program as adminstrator.
- Please make sure to run on single instance, terminate this program by Task Manager before restart this program.
- Some key might interfere with how MyWinTiles behaves, Use [AutoHotkey](https://autohotkey.com/) with [disable key script](interfere_keys.ahk)
(Run as Startup is recommend)
- Right now, each Workspace only support 20 window and there is only 10 workspace.
So, you can manage window up to 200 window per desktop.

# License
- [GNU GPLv3](LICENSE)

