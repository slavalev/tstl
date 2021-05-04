if exist ..\set_env.bat call ..\set_env.bat
if exist  .\set_env.bat call  .\set_env.bat

set path=%ddk_root%\bin\x86;%path%

nmake -nologo vcs

