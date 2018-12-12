workspace "SprayFix"
   configurations { "Release" }
   location "build"

project "SprayFix"
   kind "SharedLib"
   language "C++"
   targetdir "bin/%{cfg.buildcfg}"
   includedirs {"../sourcesdk/common", "../sourcesdk/public/tier0", "../sourcesdk/public/tier1", "../sourcesdk/public", "SprayFix"}
   architecture "x86"
   
   filter "system:Windows"
      libdirs {"../sourcesdk/lib/public"}
	  ignoredefaultlibraries { "LIBCMT" }
	  characterset "MBCS"
	  defines {"WIN32"}
	  targetname "gmsv_sprayfix_win32"
	  links {"tier0", "tier1"}
	  
   filter "system:Linux"
      targetname "gmsv_sprayfix_linux"
	  libdirs {"../sourcesdk/lib/public/linux32"}
	  implibprefix ""
      implibextension ""
	  links {"tier0", ":tier1.a"}
	  targetprefix""
	  defines {"GNUC", "POSIX", "_LINUX", "LINUX"}
	  buildoptions{ "-std=c++11 -fPIC -m32" }
	  
   targetextension ".dll"
   files { "SprayFix/*", "SprayFix/detours/hde.cpp", "**.h" }
   defines { "GMOD_USE_SOURCESDK"}

   filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "On"

   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "On"
