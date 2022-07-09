
require "bin/setup"

--============================================================================
-- ソリューションファイル
--============================================================================
config_solution("BaseProject")
    startproject "BaseProject"		-- スタートアッププロジェクト名

	-- プリプロセッサ #define
   	defines {
	--	"_ITERATOR_DEBUG_LEVEL=0",	-- イテレーターのデバッグを高速化(他のライブラリと干渉する場合は削除)
	}

	buildoptions {
		"/permissive-",
	}
	
	-- リンカー警告抑制
	linkoptions {
	}

	filter "configurations:Debug"
	   	defines {
		}
	filter "configurations:Release"
	   	defines {
		}

	filter ""
	
--============================================================================
-- 外部のプロジェクトファイル
--============================================================================
group "OpenSource"

-----------------------------------------------------------------
-- hlslpp
-----------------------------------------------------------------
config_project("hlsl++", "StaticLib")

	local SOURCE_PATH = "opensource/hlslpp"

	files
	{
		path.join(SOURCE_PATH, "include/**.h"),
		path.join(SOURCE_PATH, "include/*.natvis"),
	}

-----------------------------------------------------------------
-- ImGui
-----------------------------------------------------------------
config_project("ImGui", "StaticLib")

	characterset      "UNICODE"		-- UNICODE文字セット
   	warnings "Default"

	local SOURCE_PATH = "opensource/imgui"
	local IMGUI_PATH  = "opensource/imgui"

	-- 追加するソースコード
	-- *=フォルダ内 **=フォルダ内とその階層下サブフォルダ内
    	files {
		path.join(SOURCE_PATH, "*.cpp"),
		path.join(SOURCE_PATH, "*.h"),
		path.join(SOURCE_PATH, "*.inl"),
		path.join(SOURCE_PATH, "include/**.natvis"),

		path.join(SOURCE_PATH, "backends/imgui_impl_dx9.*"),
		path.join(SOURCE_PATH, "backends/imgui_impl_dx11.*"),
		path.join(SOURCE_PATH, "backends/imgui_impl_win32.*"),
	}
	
	-- "" インクルードパス
	includedirs {
		SOURCE_PATH,
	}

	-- 除去するファイル
	removefiles {
	}

	-- プリプロセッサ #define
   	defines {
		"_WIN32_WINNT=0x0A00",						-- _WIN32_WINNT_WIN10 (0x0A00)
	}

	-- フォルダ分け
	vpaths {
		["ヘッダー ファイル/*"] = {
			path.join(SOURCE_PATH, "**.h"),
			path.join(SOURCE_PATH, "**.hxx"),
			path.join(SOURCE_PATH, "**.hpp"),
			path.join(SOURCE_PATH, "**.inl")
		},
		["ソース ファイル/*"] = {
			path.join(SOURCE_PATH, "**.c"),
			path.join(SOURCE_PATH, "**.cxx"),
			path.join(SOURCE_PATH, "**.cpp")
		},
	}

-----------------------------------------------------------------
-- ImGuizmo
-----------------------------------------------------------------
config_project("ImGuizmo", "StaticLib")

	local SOURCE_PATH = "opensource/ImGuizmo"

   	warnings "Default"

	-- 追加するソースコード
	-- *=フォルダ内 **=フォルダ内とその階層下サブフォルダ内
    	files {
		path.join(SOURCE_PATH, "*.c"),
		path.join(SOURCE_PATH, "*.cpp"),
		path.join(SOURCE_PATH, "*.h"),
	}
	
	-- "" インクルードパス
	includedirs {
		SOURCE_PATH,
		"opensource",
	}

	-- 除去するファイル
	removefiles {
	}

	-- プリプロセッサ #define
   	defines {
	}

	-- フォルダ分け
	vpaths {
		["ヘッダー ファイル/*"] = {
			path.join(SOURCE_PATH, "**.h"),
			path.join(SOURCE_PATH, "**.hxx"),
			path.join(SOURCE_PATH, "**.hpp"),
			path.join(SOURCE_PATH, "**.inl")
		},
		["ソース ファイル/*"] = {
			path.join(SOURCE_PATH, "**.c"),
			path.join(SOURCE_PATH, "**.cxx"),
			path.join(SOURCE_PATH, "**.cpp")
		},
	}

-----------------------------------------------------------------
-- implot
-----------------------------------------------------------------
config_project("implot", "StaticLib")

	local SOURCE_PATH = "opensource/implot"

   	warnings "Default"

	-- 追加するソースコード
	-- *=フォルダ内 **=フォルダ内とその階層下サブフォルダ内
    	files {
		path.join(SOURCE_PATH, "*.c"),
		path.join(SOURCE_PATH, "*.cpp"),
		path.join(SOURCE_PATH, "*.h"),
	}
	
	-- "" インクルードパス
	includedirs {
		SOURCE_PATH,
		"opensource",
		IMGUI_PATH,
	}

	-- 除去するファイル
	removefiles {
	}

	-- プリプロセッサ #define
   	defines {
	}

	-- フォルダ分け
	vpaths {
		["ヘッダー ファイル/*"] = {
			path.join(SOURCE_PATH, "**.h"),
			path.join(SOURCE_PATH, "**.hxx"),
			path.join(SOURCE_PATH, "**.hpp"),
			path.join(SOURCE_PATH, "**.inl")
		},
		["ソース ファイル/*"] = {
			path.join(SOURCE_PATH, "**.c"),
			path.join(SOURCE_PATH, "**.cxx"),
			path.join(SOURCE_PATH, "**.cpp")
		},
	}

-----------------------------------------------------------------
-- sigslot
-----------------------------------------------------------------
config_project("sigslot", "StaticLib")

	local SOURCE_PATH = "opensource/sigslot"

	files
	{
		path.join(SOURCE_PATH, "*.hpp"),
	}

-----------------------------------------------------------------
-- meshoptimizer
-----------------------------------------------------------------
config_project("meshoptimizer", "StaticLib")

	local SOURCE_PATH = "opensource/meshoptimizer/src"

   	warnings "Default"

	-- 追加するソースコード
	-- *=フォルダ内 **=フォルダ内とその階層下サブフォルダ内
    	files {
		path.join(SOURCE_PATH, "*.c"),
		path.join(SOURCE_PATH, "*.cpp"),
		path.join(SOURCE_PATH, "*.h"),
	}
	
	-- "" インクルードパス
	includedirs {
		SOURCE_PATH,
		"opensource",
	}

	-- 除去するファイル
	removefiles {
	}

	-- プリプロセッサ #define
   	defines {
	}

	-- フォルダ分け
	vpaths {
		["ヘッダー ファイル/*"] = {
			path.join(SOURCE_PATH, "**.h"),
			path.join(SOURCE_PATH, "**.hxx"),
			path.join(SOURCE_PATH, "**.hpp"),
			path.join(SOURCE_PATH, "**.inl")
		},
		["ソース ファイル/*"] = {
			path.join(SOURCE_PATH, "**.c"),
			path.join(SOURCE_PATH, "**.cxx"),
			path.join(SOURCE_PATH, "**.cpp")
		},
	}

-----------------------------------------------------------------
-- JoltPhysics
-----------------------------------------------------------------
config_project("JoltPhysics", "StaticLib")

	local SOURCE_PATH = "opensource/JoltPhysics/Jolt"

   	warnings "Default"
	inlining "Auto"			-- 常にインライン展開
	optimize "Full"			-- 常に最適化

	buildoptions {
		"/Zo",	-- 最適化されたデバッグ機能の強化
	}
	
	-- 追加するソースコード
	-- *=フォルダ内 **=フォルダ内とその階層下サブフォルダ内
    	files {
		path.join(SOURCE_PATH, "**.c"),
		path.join(SOURCE_PATH, "**.cpp"),
		path.join(SOURCE_PATH, "**.h"),
		path.join(SOURCE_PATH, "**.natvis"),
	}
	
	-- "" インクルードパス
	includedirs {
		SOURCE_PATH,
		"opensource",
		"opensource/JoltPhysics",
	}

	-- プリコンパイル済ヘッダー
	pchheader "Jolt.h"
	pchsource (path.join(SOURCE_PATH, "pch.cpp"))
	forceincludes "Jolt.h"

	-- 除去するファイル
	removefiles {
	}

	-- プリプロセッサ #define
   	defines {
	}

	-- フォルダ分け
	vpaths {
		["ヘッダー ファイル/*"] = {
			path.join(SOURCE_PATH, "**.h"),
			path.join(SOURCE_PATH, "**.hxx"),
			path.join(SOURCE_PATH, "**.hpp"),
			path.join(SOURCE_PATH, "**.inl")
		},
		["ソース ファイル/*"] = {
			path.join(SOURCE_PATH, "**.c"),
			path.join(SOURCE_PATH, "**.cxx"),
			path.join(SOURCE_PATH, "**.cpp")
		},
		["*"] = {
			path.join(SOURCE_PATH, "**.natvis")
		},
	}

-----------------------------------------------------------------
-- cereal
-----------------------------------------------------------------
config_project("cereal", "StaticLib")

	local SOURCE_PATH = "opensource/cereal"

	files
	{
		path.join(SOURCE_PATH, "include/**.hpp"),
	}

--============================================================================
-- プロジェクトファイル
--============================================================================
group ""
config_project("BaseProject", "WindowedApp")

	local SOURCE_PATH = "src"
	local DXLIB_PATH  = "dxlib"
	local INI_PATH    = "data"
	local SAMPLE_PATH = "src/Sample"
	local DATA_PATH = "data"

	debugdir  "."		-- 実行開始時のカレントディレクトリ

    nuget {
		'AssimpCpp:5.0.1.6',	-- Assimp
	}
    
	-- 追加するソースコード
	-- *=フォルダ内 **=フォルダ内とその階層下サブフォルダ内
    	files {
		path.join(DXLIB_PATH, "**.h"),
		path.join(INI_PATH,   "**.ini"),

		path.join(SOURCE_PATH, "**.h"),
		path.join(SOURCE_PATH, "**.inl"),
		path.join(SOURCE_PATH, "**.cpp"),

		path.join(DATA_PATH, "**.h"),
		path.join(DATA_PATH, "**.fx"),
		path.join(DATA_PATH, "**.hlsl"),
		path.join(DATA_PATH, "**.hlsli"),
	}
	
	-- "" インクルードパス
	includedirs {
		SOURCE_PATH,
		DXLIB_PATH,				-- DXライブラリ Effekseer
		IMGUI_PATH,				-- ImGui
		"opensource",			-- オープンソース
		"opensource/cereal/include",
		"opensource/JoltPhysics",
		SAMPLE_PATH,
		
	}

	-- ライブラリディレクトリ
	libdirs {
		DXLIB_PATH,					-- DXライブラリ Effekseer
	}

	-- 依存ライブラリ・プロジェクト
	links {
	}

	-- プリプロセッサ #define
   	defines {
	--	"_CRT_SECURE_NO_WARNINGS",
	"_DISABLE_EXTENDED_ALIGNED_STORAGE",
	--"USE_JOLT_PHYSICS",
	}

	-- プリコンパイル済ヘッダー
	pchheader "Precompile.h"
	pchsource (path.join(SOURCE_PATH, "Precompile.cpp"))
	forceincludes "Precompile.h"

	-- フォルダ分け
	vpaths {
		["ヘッダー ファイル/*"] = {
			path.join(SOURCE_PATH, "**.h"),
			path.join(SOURCE_PATH, "**.hxx"),
			path.join(SOURCE_PATH, "**.hpp"),
			path.join(SOURCE_PATH, "**.inl")
		},
		["ソース ファイル/*"] = {
			path.join(SOURCE_PATH, "**.c"),
			path.join(SOURCE_PATH, "**.cxx"),
			path.join(SOURCE_PATH, "**.cpp")
		},
		["シェーダー ファイル/*"] = {
			path.join(DATA_PATH, "**.h"),
			path.join(DATA_PATH, "**.fx"),
			path.join(DATA_PATH, "**.hlsl"),
			path.join(DATA_PATH, "**.hlsli")
		},
		["ini ファイル/*"] = {
			path.join(INI_PATH, "**.ini")
		},
	}

	links {
		"ImGui",
		"ImGuizmo",
		"implot",
		"JoltPhysics",
		"meshoptimizer",
	}
