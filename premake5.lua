
local action = _ACTION or ""

solution "battles"
	location ( "build" )
	configurations { "Debug", "Release" }
	platforms {"x64"}
        startproject "battles"
	
   	project "nanovg"
		language "C"
		kind "StaticLib"
		targetdir("build")
		includedirs {
                    "lib/nanovg/src",
                    "lib/glfw-3.2.1.bin.WIN64/include/GLFW",
                    "lib/glew-2.0.0/include",
                }
                libdirs {
                    "lib/glfw-3.2.1.bin.WIN64/lib-vc2015",
                    "lib/glew-2.0.0/lib/Release/x64",
                }
		files {
                    "lib/nanovg/src/*.c",
                    "lib/nanovg/src/*.h"
                }
		defines { "_CRT_SECURE_NO_WARNINGS" } --,"FONS_USE_FREETYPE" } Uncomment to compile with FreeType support
		
		configuration "Debug"
			defines { "DEBUG" }
                        symbols "On"
			flags { "ExtraWarnings"}

		configuration "Release"
			defines { "NDEBUG" }
                        symbols "Off"
			flags { "Optimize", "ExtraWarnings"}

	project "battles"
		kind "ConsoleApp"
		language "C++"
		targetdir("build")
                flags { "NoNativeWChar" }
                exceptionhandling "Off"
		files {
                    "todo.txt",
                    "src/*.cpp",
                    "src/*.h",
                    "imgui/*.cpp",
                    "lib/glew-2.0.0/include/GL/*.h",
                    "lib/SDL2-2.0.5/include/*.h",
                }
		includedirs {
                    "src",
                    "imgui",
                    "lib/glfw-3.2.1.bin.WIN64/include/GLFW",
                    "lib/glew-2.0.0/include",
                    "lib/imgui",
                    "lib/nanovg/src",
                    "lib/SDL2-2.0.5/include",
                }
                libdirs {
                    "lib/glfw-3.2.1.bin.WIN64/lib-vc2015",
                    "lib/glew-2.0.0/lib/Release/x64",
                    "lib/SDL2-2.0.5/lib/x64",
                }
		links { "nanovg" }

		configuration { "windows" }
			 links {
                             "SDL2", "SDL2main", "glew32", "glfw3",
                             "gdi32", "winmm", "user32", "glu32","opengl32", "kernel32"
                         }
			 defines { "NANOVG_GLEW", "_CRT_SECURE_NO_WARNINGS" }
                         postbuildcommands {
                            "copy ..\\lib\\SDL2-2.0.5\\lib\\x64\\SDL2.dll SDL2.dll",
                            "copy ..\\lib\\glew-2.0.0\\bin\\Release\\x64\\glew32.dll glew32.dll",
                         }

		configuration "Debug"
			defines { "DEBUG" }
                        symbols "On"
                        editAndContinue "On"
			flags { "ExtraWarnings"}

		configuration "Release"
			defines { "NDEBUG" }
                        symbols "Off"
			flags { "Optimize", "ExtraWarnings"}

			 defines { "NANOVG_GLEW" }

