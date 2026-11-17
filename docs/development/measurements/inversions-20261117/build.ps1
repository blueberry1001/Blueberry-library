$msvc = 'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC\14.43.34808'
$sdk = 'C:\Program Files (x86)\Windows Kits\10'
$ver = '10.0.22621.0'
$clang = 'C:\Program Files\Unity\Hub\Editor\6000.5.2f1\Editor\Data\PlaybackEngines\AndroidPlayer\NDK\toolchains\llvm\prebuilt\windows-x86_64\bin\clang++.exe'
$env:INCLUDE = "$msvc\include;$sdk\Include\$ver\ucrt;$sdk\Include\$ver\shared;$sdk\Include\$ver\um"
$env:LIB = "$msvc\lib\x64;$sdk\Lib\$ver\ucrt\x64;$sdk\Lib\$ver\um\x64"
$env:PATH = "$msvc\bin\Hostx64\x64;$env:PATH"
& $clang --version
& $clang --target=x86_64-pc-windows-msvc -fuse-ld=link -std=gnu++20 -O2 -I. .build/inversions-experiment/test.cpp -o .build/inversions-experiment/test.exe
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
& .build/inversions-experiment/test.exe
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
& $clang --target=x86_64-pc-windows-msvc -fuse-ld=link -std=gnu++20 -O2 -DNDEBUG -I. .build/inversions-experiment/bench.cpp -o .build/inversions-experiment/bench.exe
exit $LASTEXITCODE
