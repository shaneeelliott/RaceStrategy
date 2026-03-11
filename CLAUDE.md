# Nautograf — Claude Code Instructions

## Build

Building from a bash shell requires MSVC standard library paths to be set manually
(vcvars64.bat does not propagate through `cmd /c` from bash).

```bash
MSVC_INC="C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.44.35207/include"
UCRT_INC="C:/Program Files (x86)/Windows Kits/10/Include/10.0.26100.0/ucrt"
UM_INC="C:/Program Files (x86)/Windows Kits/10/Include/10.0.26100.0/um"
SHARED_INC="C:/Program Files (x86)/Windows Kits/10/Include/10.0.26100.0/shared"
MSVC_LIB="C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.44.35207/lib/x64"
UCRT_LIB="C:/Program Files (x86)/Windows Kits/10/Lib/10.0.26100.0/ucrt/x64"
UM_LIB="C:/Program Files (x86)/Windows Kits/10/Lib/10.0.26100.0/um/x64"

export INCLUDE="$MSVC_INC;$UCRT_INC;$UM_INC;$SHARED_INC"
export LIB="$MSVC_LIB;$UCRT_LIB;$UM_LIB"

# Build a specific target (e.g. scene, tilefactory, nautograf)
cd "C:/dev/RaceStrategy/builds/ninja-debug-vcpkg" && ninja scene

# Build everything
cd "C:/dev/RaceStrategy/builds/ninja-debug-vcpkg" && ninja

# After linking scene.dll, copy it to the executable directory
cp src/scene/scene.dll ./scene.dll
```

The active build preset is `ninja-debug-vcpkg` (Debug, Ninja, vcpkg).
Build output is at `builds/ninja-debug-vcpkg/`.

## Run with Logging

```bash
powershell -Command "
\$p = Start-Process \
  -FilePath 'C:\dev\RaceStrategy\builds\ninja-debug-vcpkg\nautograf.exe' \
  -WorkingDirectory 'C:\dev\RaceStrategy\builds\ninja-debug-vcpkg' \
  -RedirectStandardError 'C:\Temp\nautograf_run.log' \
  -PassThru -NoNewWindow
Start-Sleep -Seconds 5
if (-not \$p.HasExited) {
    'Running - PID: ' + \$p.Id + ', Window: ' + \$p.MainWindowTitle
} else {
    'Crashed with exit code: ' + \$p.ExitCode
}
"

# Read log after testing
cat /c/Temp/nautograf_run.log
```

## Verify Fixes After Changes

After rebuilding, run the app and check the log contains only expected output:

```bash
# Expected (pre-existing, non-fatal):
#   Unrecognized Record: 58236
#   Failed to read
#
# NOT expected (would indicate regression):
#   QSGNode: Use of Qt::DirectConnection in graphics thread
#   any crash / exit code != 0
```

To confirm the zoom flicker fix specifically:
1. Launch the app and load a chart directory
2. Zoom in and out repeatedly using the scroll wheel
3. There should be **no gray grid** and **no white flash** between zoom levels
4. Tiles should hold their previous appearance until new tile data is ready

## Known Warnings (Non-Fatal)

- `Unrecognized Record: 58236` — OSENC chart file contains a record type unknown to the
  current oesenc library version. The chart is partially skipped. Not caused by our code.
- `applocal.ps1: Neither dumpbin ... could be found` — vcpkg's DLL dependency helper
  can't run without dumpbin on PATH. DLLs are already deployed via windeployqt; harmless.

## Qt Deploy (first time or after Qt upgrade)

If Qt DLLs are missing from the build directory:

```bash
"C:/Qt/6.7.2/msvc2019_64/bin/windeployqt.exe" \
  --no-compiler-runtime \
  --qmldir "C:/dev/RaceStrategy/qml" \
  "C:/dev/RaceStrategy/builds/ninja-debug-vcpkg"
```
