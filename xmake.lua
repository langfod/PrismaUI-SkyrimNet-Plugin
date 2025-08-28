-- set minimum xmake version
set_xmakever("2.8.2")

includes("lib/commonlibsse-ng")

set_project("PrismaUI-SkyrimNet-UI")
set_version("0.2.0-beta")
set_license("GPL-3.0")

set_languages("c++23")
set_warnings("allextra")

set_policy("package.requires_lock", true)

add_rules("mode.release")
add_rules("plugin.vsxmake.autoupdate")

-- targets
target("PrismaUI-SkyrimNet-UI")
    add_deps("commonlibsse-ng")

    add_rules("commonlibsse-ng.plugin", {
       name = "PrismaUI-SkyrimNet-UI",
       author = "StarkMP <discord: starmp>",
       description = "SKSE64 plugin template using CommonLibSSE-NG and PrismaUI"
    })

    add_files("src/**.cpp")
    add_headerfiles("src/**.h")
    add_includedirs("src")
    set_pcxxheader("src/pch.h")
    set_symbols("debug") -- debug

        after_build(function (target)
        import("core.project.project")
        local build_dir = target:targetdir()
        local release_dir = path.join(project.directory(), "release", target:basename())

        local dll_name = target:basename() .. ".dll"
        local dll_path = path.join(build_dir, dll_name)
        local pdb_name = target:basename() .. ".pdb" -- debug
        local pdb_path = path.join(build_dir, pdb_name) -- debug
        local view_path = path.join(project.directory(), "view", "index.html")

        local release_plugins_dir = path.join(release_dir,"SKSE","Plugins")
        local release_views_dir = path.join(release_dir, "PrismaUI", "views", target:basename())

        os.rm(path.join(project.directory(), "release"), {rootdir = path.join(project.directory(), "release")})

        os.mkdir(release_plugins_dir)
        os.cp(dll_path, release_plugins_dir) -- debug
        os.cp(pdb_path, release_plugins_dir)

        os.mkdir(release_views_dir)
        os.cp(view_path, release_views_dir)

        -- Create 7z archives
        import("utils.archive")
        local options = {}
        options.recurse = true
        archive.archive(release_dir .. ".7z", release_dir .. "/**", options)
    end)