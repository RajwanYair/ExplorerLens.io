**V1.7 is released!**

Read the details and download the installer from the [releases page](https://github.com/fire-eggs/DarkThumbs/releases/tag/V1.7).
Also see the releases page for additional changes in earlier releases.

*Status*

Due to a family crisis, the latter half of 2022 was a complete loss for working on projects.

With the New Year 2023, I hope to move forward again on this and other projects. I hope for your understanding and patience.

# DarkThumbs
Thumbnail preview support for ebooks and various archive formats in Windows File Explorer.

## System Requirements
**Windows 10/11 64-bit ONLY** (Build 19041 or later)

> **Note:** This project is optimized exclusively for 64-bit Windows with maximum performance. 32-bit support has been removed in favor of modern optimizations.

**Build Requirements:**
- Visual Studio Build Tools 2022 (v143 platform toolset)
- Windows 11 SDK (10.0.22621.0 or later)
- C++17 Standard with maximum optimizations (/O2 /Ot /Oi /GL /LTCG)
- WTL 9.1 (installed via NuGet)

**Runtime Requirements:**
[Visual C++ Redistributable for Visual Studio 2022 (x64)](https://aka.ms/vs/17/release/vc_redist.x64.exe)

**Quick Install:**
```cmd
# Download latest release, extract, then:
install-x64.cmd  # Run as Administrator
```

**Build from Source:**
```cmd
# Install VS Build Tools 2022, then:
build-release-x64.cmd
```

See [INSTALLATION_GUIDE.md](INSTALLATION_GUIDE.md) for complete installation and build instructions.

## Screenshot
(See the [V1.7 release](https://github.com/fire-eggs/DarkThumbs/releases/tag/V1.7) for a view of the current manager UI)
![V1.5](DarkThumbs15_demo.png)

## Supported formats
- EPUB
- FB2
- Kindle (MOBI, AZW, AZW3)
- CBZ
- CBR
- RAR
- ZIP

**With a Little Help From ...**

![logo2](https://github.com/fire-eggs/yagp/blob/master/Files/deleaker_logo.png) - [Deleaker](https://www.deleaker.com) : the _best_ tool for finding memory, GDI and other leaks!

## Credits

### Source Code
Used [Invertex's CBXShell](https://github.com/Invertex/CBXShell) as a base

MOBI support thanks to [libmobi](https://github.com/bfabiszewski/libmobi)

### Application Icon
Icons made by [Those Icons](https://www.flaticon.com/authors/those-icons) from [Flaticon](https://www.flaticon.com/)

- Sponsored by [L0garthimic](https://github.com/L0garithmic)
- V1.0 by [dark-knight404](https://github.com/dark-knight404)
- V1.1-V1.7 by [fire-eggs](https://github.com/fire-eggs)
