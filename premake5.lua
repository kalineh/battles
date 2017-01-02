
local action = _ACTION or ""

solution "battles"
	location ( "build" )
	configurations { "Debug", "Release" }
	platforms {"x64"}
	
   	project "nanovg"
		language "C"
		kind "StaticLib"
		targetdir("build")
		includedirs {
                    "nanovg/src",
                    "glfw-3.2.1.bin.WIN64/include/GLFW",
                    "glew-2.0.0/include",
                }
                libdirs {
                    "glfw-3.2.1.bin.WIN64/lib-vc2015",
                    "glew-2.0.0/lib/Release/x64",
                }
		files { "nanovg/src/*.c" }
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
		language "C"
		targetdir("build")
		files { "src/*.cpp" }
		includedirs {
                    "src",
                    "glfw-3.2.1.bin.WIN64/include/GLFW",
                    "glew-2.0.0/include",
                }
                libdirs {
                    "glfw-3.2.1.bin.WIN64/lib-vc2015",
                    "glew-2.0.0/lib/Release/x64",
                }
		links { "nanovg" }

		configuration { "windows" }
			 links { "glfw3", "gdi32", "winmm", "user32", "glew32s", "glu32","opengl32", "kernel32" }
			 defines { "NANOVG_GLEW", "_CRT_SECURE_NO_WARNINGS" }

		configuration "Debug"
			defines { "DEBUG" }
                        symbols "On"
			flags { "ExtraWarnings"}

		configuration "Release"
			defines { "NDEBUG" }
                        symbols "Off"
			flags { "Optimize", "ExtraWarnings"}

			 defines { "NANOVG_GLEW" }

		configuration { "windows" }
			 links { "glfw3", "gdi32", "winmm", "user32", "glew32s", "glu32","opengl32", "kernel32" }

