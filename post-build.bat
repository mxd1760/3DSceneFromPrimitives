if %3 == "debug" (
  set extra=debug\
) else (
  set "extra=" 
)

robocopy %1\vcpkg_installed\x64-windows\%extra%bin %2 *.dll
robocopy %1\obj_files %2\obj_files /e
robocopy %1\shaders  %2\shaders 
robocopy %1\textures %2\textures 

echo finished successfully 