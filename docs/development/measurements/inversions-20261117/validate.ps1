$msvc = 'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC\14.43.34808'
$sdk = 'C:\Program Files (x86)\Windows Kits\10'
$ver = '10.0.22621.0'
$clang = 'C:\Program Files\Unity\Hub\Editor\6000.5.2f1\Editor\Data\PlaybackEngines\AndroidPlayer\NDK\toolchains\llvm\prebuilt\windows-x86_64\bin\clang++.exe'
$env:INCLUDE = "$msvc\include;$sdk\Include\$ver\ucrt;$sdk\Include\$ver\shared;$sdk\Include\$ver\um"
$env:LIB = "$msvc\lib\x64;$sdk\Lib\$ver\ucrt\x64;$sdk\Lib\$ver\um\x64"
$env:PATH = "$msvc\bin\Hostx64\x64;$env:PATH"
foreach ($std in @('gnu++20', 'gnu++23')) {
  foreach ($name in @('static-range-inversions-sparse', 'sequence-batch-four')) {
    Write-Output "compile $std $name"
    & $clang --target=x86_64-pc-windows-msvc -fuse-ld=link "-std=$std" -O2 -Wall -Wextra -Wshadow -Werror -I. "tests/random/$name.cpp" -o ".build/inversions-experiment/$name.exe"
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    foreach ($seed in @(1,2,17,20261117)) {
      & ".build/inversions-experiment/$name.exe" $seed
      if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
      Write-Output "pass $std $name seed=$seed"
    }
  }
}
