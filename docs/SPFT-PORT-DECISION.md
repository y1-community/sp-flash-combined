# SP Flash Tool: V5 vs V6 as the base for a macOS / ARM64 port — decision document

Scope: should we base the next effort on **SP Flash Tool V6** (Qt5, `SP_Flash_Tool_v6`,
SPFlashToolV6.pro) supplemented with a built-in `scatter.txt` converter, rather than V5
(`SP-Flash-Tool-src`, SPFlashToolAPCore.pro)? Every claim below cites the file it was read from.

---

## 1. Ground truth

### 1.1 V5 — `/Users/ryan/Documents/GitHub/SP-Flash-Tool-src`

| Aspect | Fact | Evidence |
|---|---|---|
| Language / framework | C++ **Qt 4.8 or lower** | `README.md`: "Project can be build with QT Lib 4.8 or lower"; `SPFlashToolAPCore.pro` |
| Qt modules | `QT += xmlpatterns`, `QT += webkit network`; widgets implicit (Qt4 `gui`) | `SPFlashToolAPCore.pro` |
| Qt4-only APIs | `QTextCodec::setCodecForTr()` / `setCodecForCStrings()` | `README.md`; `Utility/Utils.h:114-116` |
| WebKit GUI widget | `QWebView` promoted widget | `UI/forms/TabWidget/WelcomeWidget.ui` (`<header>QtWebKit/QWebView</header>`) |
| Build system | qmake via GNU `make`; `make HOST=(linux/windows) BUILD_TYPE=(debug/release)` | `Makefile`; `Build/build-linux.mk`, `Build/build-windows.mk` |
| Platform abstraction | Two host back-ends only: `Host/Linux/*`, `Host/Win/*`; selected by `unix:`/`win32:` in the project file | `SPFlashToolAPCore.pro` (`unix:SOURCES += Host/Linux/...`, `win32:SOURCES += Host/Win/...`); `Host/Linux/DeviceScan.cpp` uses Linux `netlink` uevent |
| **Engine linkage** | **Closed binaries**, no source. Linux: `libflashtool.so`, `libflashtoolEx.so`, `libsla_challenge.so`; Windows: `FlashToolLib.dll/.lib`, `FlashtoollibEx.dll/.lib`, `SLA_Challenge.dll` | `SPFlashToolAPCore.pro` (`unix:LIBS += -lflashtool -lflashtoolEx -lsla_challenge`); `Lib/` |
| Engine ABI size | `libflashtool.so` = ELF 64-bit x86-64, exports **842** symbols; GUI references **297** distinct engine entry points | `file Lib/libflashtool.so`; `nm -D --defined-only`; symbol scan of GUI sources |
| **Scatter handling** | The GUI does **not** parse `scatter.txt`. It hands the file to the closed engine | `Resource/Handle/DLHandle.cpp:39` → `DL_LoadScatter(handle_t_, scat_file.c_str(), NULL)`; `Resource/CoreHandle.cpp:104 LoadScatter()` |
| macOS support | **None.** No `__APPLE__`/Darwin/Mach-O branch anywhere | case-insensitive search for `__APPLE__|Darwin|macOS` across the tree → 0 matches |

### 1.2 V6 — `/Users/ryan/Documents/GitHub/SP_Flash_Tool_v6` (imported "v6.2228")

| Aspect | Fact | Evidence |
|---|---|---|
| Language / framework | C++ **Qt5** (`greaterThan(QT_MAJOR_VERSION, 4): QT += widgets`), `CONFIG += c++11` | `SPFlashToolV6.pro` |
| Qt modules | `core gui xml xmlpatterns serialport concurrent` — **no WebKit** | `SPFlashToolV6.pro:1` |
| WebKit GUI widget | Already replaced by `QTextBrowser` | `UI/Forms/WelcomeWidget.ui:36` |
| Build system | `make.sh [release/debug]` → `make HOST=(linux/windows)`; qmake path hard-coded to Qt 5.14.2 | `make.sh`; `Makefile`; `Build/defines-linux.mk` (`QMAKE := /opt/Qt5.14.2/5.14.2/gcc_64/bin/qmake`) |
| Platform abstraction | `Host/Linux/*`, `Host/Win/*`; platform macros `_LINUX64` (locked on by the `unix:` scope) and `_WIN64`/`_WIN32` | `Host/Inc/RuntimeMemory.h`; `SPFlashToolV6.pro:20` (`unix: { DEFINES += "_LINUX64" ... }`); `UI/Src/main.cpp:38,75` |
| **Engine linkage** | **Closed binaries**, no source. Linux: `libflash.1.0.0.so`, `libimagechecker.1.0.0.so`, `libsla_challenge.so`; Windows: `flash.dll`, `imageChecker.dll`, `SLA_Challenge.dll` | `SPFlashToolV6.pro:46` (`unix:LIBS += -lflash.1.0.0 -limagechecker.1.0.0 -lsla_challenge`); `Libs/`; `file` shows ELF x86-64 |
| **README self-declaration** | "libflash is provided as **x86_64 binary blob**, so there is no chance to build **ARM64** version of the tool." | `README.md` |
| Engine ABI size | **10** `flashtool_*` functions + **2** image-checker functions + SLA | `Inc/Flash.Mode.API.h`; `Inc/images_checker.h` |
| Engine is command-driven | GUI generates XML command strings (`CMD:WRITE-PARTITION`, `CMD:FLASH-ALL`, …) and passes them to `flashtool_execute_command()` | `XML/XMLConst.h`; `Inc/Flash.Mode.API.h`; `Inc/CFlashToolAPI.h/.cpp`; `XML/CmdGenerator/*.cpp` |
| Platform guard rejects non-Win/Linux | Compile-time `#error` if neither `_WIN64`/`_WIN32` nor `_LINUX64` | `Inc/Flash.Mode.API.h`, `Inc/Flash.Mode.Struct.h`, `Inc/images_checker.h`, `Inc/error_code.h` (`#error "win64 or linux64 platform is not specified."`) |
| **Input format** | `download_agent/flash.xml` (selected in the "Download-XML" field) → references a companion **scatter XML** → partition list | `XML/Parser/FlashXMLParser.cpp` (reads `<scatter>`, `<contact>`, phone-tracking; validates `flash.xsd`); `XML/Parser/ScatterXMLParser.cpp` (reads `<general><platform>`, `<storage_type name=…>`, `<partition_index>`); `XML/XMLConst.h` (`SCATTER_NODE_NAME "scatter"`); `UI/Src/AsyncFlashXMLLoader.cpp` |
| "rawprogram" term | **Absent.** No occurrence of `rawprogram` anywhere in V6 | case-insensitive search → 0 matches |

---

## 2. Answers

### Q1 — Can V6 be built from source and actually run on macOS Big Sur+ (Intel & ARM64), Windows and Linux?

**Windows x86_64 and Linux x86_64: yes.** They are the two supported targets and ship the
matching x86_64 engine blobs (`Libs/*.dll`, `Libs/*.so`, `SPFlashToolV6.pro`).

**macOS (Intel or ARM64): no — and building from source does not change that.** The blocker is
not the GUI, it is the engine: there is **no Mach-O `libflash`**; the only engine artifacts are
an ELF `libflash.1.0.0.so` and a PE `flash.dll` (`Libs/`). No macOS engine binary is shipped, and
the engine is closed source, so it cannot be recompiled for macOS. The engine headers actively
reject the platform (`#error "win64 or linux64 platform is not specified."`, `Inc/Flash.Mode.API.h`).
Relinking an ELF blob into a macOS app is impossible.

**ARM64 anywhere: no** with MTK's own engine — stated verbatim in V6's `README.md`.

> Consequence: choosing V6 does **not** by itself deliver macOS/ARM64. Both V5 and V6 share the
> same structural blocker (a closed x86_64 engine with no macOS/arm64 build). What V6 changes is
> how expensive it is to *replace* the engine (see Q3).

### Q2 — Is a built-in `scatter.txt` → "rawprogram XML" converter the right way to keep pre-V6 devices flashable from the V6 GUI?

**Partly, and the framing needs correcting.** V6 does not use "rawprogram XML". Its on-disk input
is `download_agent/flash.xml`, which references a companion **scatter XML** consumed by
`ScatterXMLParser` (`XML/Parser/FlashXMLParser.cpp`, `XML/Parser/ScatterXMLParser.cpp`,
`XML/XMLConst.h`). So the correct artifact is a

> **`scatter.txt` → V6 scatter-XML (+ synthetic `flash.xml`) adapter.**

This is the right and cheap way to restore the **partition-selection UX** and the ability to open a
V5-style package in the V6 GUI, and it slots naturally into the existing parser layer.

It is **not sufficient by itself to flash pre-V6 hardware**, because device support comes from the
engine, not from the XML: `flashtool_scan_device` / `flashtool_connect_device` / `flashtool_enter_mode`
must actually speak the old BROM/DA protocol. Evidence: V5's own scatter parsing lives inside the
closed engine (`Resource/Handle/DLHandle.cpp:39`), i.e. format parsing and device protocol are
coupled in the blob. With MTK's V6 blob there is no guarantee legacy BROM is supported; with a
replacement engine the coverage is the replacement's (mtkclient is broad; penumbra/antumbra
explicitly does **not** support V3/legacy — see §3).

There is also **no public V5-scatter → V6-flash.xml converter** to borrow; the community answer has
been "use V5" (Hovatek thread-159; see Sources).

### Q3 — What does wiring antumbra/mtkclient into the macOS release actually cost, given the V5 engine binaries have no macOS build?

*On V5* it is **impractical**: the engine ABI is 297 distinct entry points over an 842-symbol blob
with ~2,000 lines of hand-written struct headers (`BootRom/flashtool_api.h`). Reimplementing that
faithfully is a project in itself.

*On V6* it is **tractable and is the recommended seam**: the engine surface is 10 functions
(`Inc/Flash.Mode.API.h`) plus two image-checker functions (`Inc/images_checker.h`) and SLA, and it
is **entirely driven by XML command strings** the GUI already builds (`XML/CmdGenerator/*.cpp`,
`Inc/CFlashToolAPI.cpp`). A macOS/ARM64 `libflash` implementation that maps
`flashtool_scan_device` → device discovery, `flashtool_connect_device`/`flashtool_enter_mode` →
BROM/DA session setup, and `flashtool_execute_command` → per-command operations onto mtkclient
(Python bridge) or antumbra (Rust) is a small, well-bounded adapter rather than a rewrite.

Cost drivers to budget:
1. Process/IPC bridge from the Qt app to the replacement engine (Python or Rust).
2. Mapping MTK's command XML (`XML/XMLConst.h`) onto the replacement API — the bulk of the work.
3. DA selection / SLA-challenge / auth-file semantics differences.
4. macOS packaging: libusb, code signing/notarization.
5. **PyInstaller assumption correction:** upstream mtkclient does **not** ship official macOS
   PyInstaller binaries (its macOS path is a source install: brew + libusb + PySide6 + pyusb/pyserial),
   so we must build the bundle ourselves. PyInstaller cannot cross-compile, so the "oldest macOS
   supported" equals the deployment target of the Python/libusb/PySide6 wheels on the build host —
   a deliberate choice, not a value we can read off upstream.

### Q4 — Is a Linux ARM64 port realistic?

**Not with MTK's shipped engine** (x86-64 ELF only; V6's README says ARM64 is impossible).
**Yes with a replacement engine:** the Qt5 GUI is architecture-independent, and the only hard gates
are the `#error` platform guards in `Inc/*.h` (add an Apple/arm64 branch) and the engine. The
`shomykohai/penumbra` project already publishes `antumbra-linux-aarch64`. macOS ARM64 and Linux
ARM64 therefore unlock with the **same** work (engine replacement + platform branches).

---

## 3. Recommendation — one path

**Base the cross-platform future on V6, but ship an *engine replacement*, not a plain source build.
Keep V5 frozen for users who need MTK's genuine engine.**

Rationale, in priority order:

1. **V6's small, XML-driven engine ABI is the only realistic replacement seam.** 10 functions vs 297.
   This is what makes macOS and ARM64 possible at all.
2. **V6's stack is already modern**: Qt5 widgets, c++11, no WebKit, uses `QTextBrowser`
   (`UI/Forms/WelcomeWidget.ui`). V5 would first need a full Qt4→Qt5 port (WebKit, `xmlpatterns`,
   `setCodecForTr`/`setCodecForCStrings`).
3. **The scatter work fits V6's existing parser layer** — add a `scatter.txt` reader and emit the
   scatter XML that `ScatterXMLParser` already consumes, plus a synthetic `flash.xml`.
4. **Dual-engine build keeps Windows/Linux "normal":** keep linking MTK's real blob on
   Windows/Linux x86-64 (unchanged) and link the replacement engine on macOS/ARM64. `CFlashToolAPI`
   (`Inc/CFlashToolAPI.h`) is the natural façade to hold both back-ends.

Explicit caveat on the goal "flash any device V5/V6 can": once the engine is replaced, device
coverage is the replacement engine's coverage. **mtkclient** is broad (includes legacy);
**penumbra/antumbra** covers V5/XFlash and V6/XML and explicitly declines V3/legacy. Exact parity
with MTK's private engine (secure-boot/SLA/auth edge cases) is not guaranteed and must be validated
device-by-device.

### First concrete implementation step

Prove the GUI is separable from the blob with the smallest decisive experiment:

1. Add a macOS branch to the engine headers — replace the `#error` in `Inc/Flash.Mode.API.h`,
   `Inc/Flash.Mode.Struct.h`, `Inc/images_checker.h`, `Inc/error_code.h` with an `__APPLE__` case
   (define `LIB_API`/`LIBEX_EXPORT` as no-ops, as the `_LINUX64` branch does).
2. Add a `macx:` scope to `SPFlashToolV6.pro` (Qt5 via Homebrew, drop `-no-pie`, link a stub
   `libflash`) and a `Host/Mac/` back-end mirroring `Host/Linux/`.
3. Build a stub `libflash` exporting the 10 functions + 2 checker functions (return
   "not implemented") and confirm the **V6 GUI compiles, links and launches on this M1 Mac**.

That establishes the platform seam and the build recipe; the mtkclient back-end then replaces the
stub function-by-function.

### Needs your decision

1. **Replacement engine:** mtkclient (Python) vs penumbra/antumbra (Rust) vs both? Coverage differs
   (penumbra = V5/V6 protocols, no legacy; mtkclient = broad incl. legacy).
2. **Dual engine vs unified:** MTK blob on Win/Linux-x86_64 with replacement only on macOS/ARM64,
   or replacement everywhere?
3. **Minimum macOS:** since mtkclient ships no PyInstaller binaries, you must pick the build host
   and deployment target (arm64 floor ≈ Big Sur 11.0; decide the Intel floor separately).
4. **Canonical repo:** fork/keep V6 as the base and archive V5, or merge V6's GUI into this repo?

---

## 4. Sources

Codebase (read directly): `SPFlashToolAPCore.pro`, `README.md`, `Makefile`, `Build/build-linux.mk`,
`Resource/Handle/DLHandle.cpp`, `Resource/CoreHandle.cpp`, `Utility/Utils.h`,
`UI/forms/TabWidget/WelcomeWidget.ui`, `Lib/*` (V5); `SPFlashToolV6.pro`, `README.md`, `make.sh`,
`Makefile`, `Build/defines-linux.mk`, `Inc/Flash.Mode.API.h`, `Inc/Flash.Mode.Struct.h`,
`Inc/images_checker.h`, `Inc/CFlashToolAPI.h`, `XML/XMLConst.h`, `XML/Parser/FlashXMLParser.cpp`,
`XML/Parser/ScatterXMLParser.cpp`, `UI/Src/AsyncFlashXMLLoader.cpp`, `UI/Forms/WelcomeWidget.ui`,
`Host/*`, `Libs/*` (V6).

Web: `shomykohai/penumbra` README + releases (Rust; `antumbra`; V5/XFlash + V6/XML; no V3/legacy;
`antumbra-linux-aarch64`, `antumbra-darwin-aarch64`); mtkclient `README-INSTALL.md` +
`requirements.txt` (macOS source install; pyusb/pyserial; no official macOS PyInstaller binaries);
Hovatek thread-159 p.41 (no V5-scatter → V6-flash.xml converter; "use v5"); Lenovo TB375FC and
UMIDIGI V6 packaging guides (`download_agent/flash.xml` + companion `MT…_Android_scatter.xml`).
