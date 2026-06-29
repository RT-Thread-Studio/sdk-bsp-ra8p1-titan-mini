@echo off
%1 mshta vbscript:CreateObject("Shell.Application").ShellExecute("cmd.exe","/c %~s0 ::","","runas",1)(window.close)&&exit
cd /d "%~dp0"
@echo on
mklink /D rt-thread ..\..\rt-thread
mklink /D libraries ..\..\libraries
mklink /D ra ..\..\FSPConfiguration\ra
mklink /D ra_cfg ..\..\FSPConfiguration\ra_cfg
mklink /D ra_gen ..\..\FSPConfiguration\ra_gen
mklink configuration.xml ..\..\FSPConfiguration\configuration.xml
