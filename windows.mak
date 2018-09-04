#	Build MSI packages

MSB=%ProgramFiles(x86)%\Microsoft Visual Studio\2017\Community\MSBuild\15.0\Bin\amd64\msbuild.exe

WIX_LINK = light.exe -ext WixUIExtension -ext WixUtilExtension -dWixUILicenseRtf=doc\soferox_lic.rtf -dWixUIDialogBmp=share\pixmaps\wix-banner.bmp -dWixUIBannerBmp=share\pixmaps\wix-topbanner.bmp


all : soferox-2.16.0-win32.msi soferox-2.16.0-win64.msi

src\qt\res_bitcoin.cpp : src\qt\moc.proj
	cd src\qt
	msbuild moc.proj
	cd ..\..


x86_R_St\soferox-qt.exe : src\soferox.cpp src\qt\res_bitcoin.cpp
	""$(MSB)"" my-soferox.sln /p:Configuration=R_St,Platform=x86 /v:n

x64_R_St\soferox-qt.exe : src\soferox.cpp src\qt\res_bitcoin.cpp
	""$(MSB)"" my-soferox.sln /p:Configuration=R_St,Platform=x64 /v:n


soferox-2.16.0-win32.msi : soferox.wxs x86_R_St\soferox-qt.exe
	candle.exe -o soferox-x86.wixobj soferox.wxs
	$(WIX_LINK)  -out $@ soferox-x86.wixobj

soferox-2.16.0-win64.msi : soferox.wxs x64_R_St\soferox-qt.exe
	candle.exe -arch x64 -o soferox-x64.wixobj soferox.wxs
	$(WIX_LINK) -out $@ soferox-x64.wixobj
