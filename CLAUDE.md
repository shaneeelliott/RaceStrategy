# Nautograf — Claude Code Instructions

## Build

MSVC environment variables must be set as Process-level vars before running cmake or
ninja. Use PowerShell scripts for this — bash `export` does NOT work because cmake
spawns cmd.exe child processes that don't inherit bash env vars.

The helper scripts below are at `C:/Temp/` (recreate them if missing):

### `C:/Temp/build.ps1` — build everything
```powershell
[System.Environment]::SetEnvironmentVariable('LIB', 'C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.44.35207\lib\x64;C:\Program Files (x86)\Windows Kits\10\Lib\10.0.26100.0\ucrt\x64;C:\Program Files (x86)\Windows Kits\10\Lib\10.0.26100.0\um\x64', 'Process')
[System.Environment]::SetEnvironmentVariable('INCLUDE', 'C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.44.35207\include;C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\ucrt;C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\um;C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\shared', 'Process')
Set-Location 'C:\dev\RaceStrategy\builds\ninja-debug-vcpkg'
$result = & 'C:\Users\shane\AppData\Local\Programs\Python\Python313\Scripts\ninja.exe' 2>&1
$result | Out-File -FilePath 'C:\Temp\ninja_build.log' -Encoding utf8
$LASTEXITCODE | Out-File -FilePath 'C:\Temp\ninja_build_exit.log' -Encoding utf8
```

```bash
# Run the build
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "C:\\Temp\\build.ps1"

# Check result
cat /c/Temp/ninja_build_exit.log   # 0 = success
tail -20 /c/Temp/ninja_build.log

# After linking scene.dll, copy it to the executable directory
cp /c/dev/RaceStrategy/builds/ninja-debug-vcpkg/src/scene/scene.dll \
   /c/dev/RaceStrategy/builds/ninja-debug-vcpkg/scene.dll
```

### `C:/Temp/configure.ps1` — re-run cmake configure (only needed after CMakeLists changes)
```powershell
[System.Environment]::SetEnvironmentVariable('LIB', 'C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.44.35207\lib\x64;C:\Program Files (x86)\Windows Kits\10\Lib\10.0.26100.0\ucrt\x64;C:\Program Files (x86)\Windows Kits\10\Lib\10.0.26100.0\um\x64', 'Process')
[System.Environment]::SetEnvironmentVariable('INCLUDE', 'C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.44.35207\include;C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\ucrt;C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\um;C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\shared', 'Process')
$result = & 'C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe' -S 'C:\dev\RaceStrategy' -B 'C:\dev\RaceStrategy\builds\ninja-debug-vcpkg' 2>&1
$result | Out-File -FilePath 'C:\Temp\cmake_configure.log' -Encoding utf8
$LASTEXITCODE | Out-File -FilePath 'C:\Temp\cmake_configure_exit.log' -Encoding utf8
```

The active build preset is `ninja-debug-vcpkg` (Debug, Ninja, vcpkg).
Build output is at `builds/ninja-debug-vcpkg/`.

## Run with Logging

Launch the app with a **visible window** so the user can see it running and interact
with it. Use `-NoNewWindow` for background stderr capture alongside a visible window:

```bash
# Launch with visible window; stderr goes to log file
powershell.exe -NoProfile -Command "
\$p = Start-Process \`
  -FilePath 'C:\dev\RaceStrategy\builds\ninja-debug-vcpkg\nautograf.exe' \`
  -WorkingDirectory 'C:\dev\RaceStrategy\builds\ninja-debug-vcpkg' \`
  -RedirectStandardError 'C:\Temp\nautograf_run.log' \`
  -PassThru
Start-Sleep -Seconds 5
if (-not \$p.HasExited) {
    'Running - PID: ' + \$p.Id
} else {
    'Crashed with exit code: ' + \$p.ExitCode
}
"

# Read log after testing
cat /c/Temp/nautograf_run.log
```

## Verify Fixes After Changes

After rebuilding, always **launch the app so the user can see it** (use the Run command
above without `-NoNewWindow` so a window appears). Then check the log:

```bash
# Expected (pre-existing, non-fatal):
#   Unrecognized Record: 58236
#   Failed to read
#
# NOT expected (would indicate regression):
#   QSGNode: Use of Qt::DirectConnection in graphics thread
#   any crash / exit code != 0
```

The app must:
1. Open a visible window (not crash silently)
2. Show the chart UI — user can then load a chart directory and inspect rendering
3. Log only the expected warnings above

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
