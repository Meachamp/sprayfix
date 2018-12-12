workspace "SprayFix"
   configurations { "Debug", "Release" }
   location "build"

project "SprayFix"
   kind "SharedLib"
   language "C++"
   targetdir "bin/%{cfg.buildcfg}"
   includedirs {"../sourcesdk/common", "../sourcesdk/public/tier0", "../sourcesdk/public/tier1", "../sourcesdk/public"}
	libdirs {"../sourcesdk/lib/public"}
	links {"tier0", "tier1"}
	ignoredefaultlibraries { "LIBCMT" }
	characterset "MBCS"

   
   files { "SprayFix/main.cpp", "SprayFix/detours/hde.cpp" }
   
   defines { "GMOD_USE_SOURCESDK", "WIN32" }
   
   targetname "gmsv_sprayfix_win32"

   filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "On"

   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "On"