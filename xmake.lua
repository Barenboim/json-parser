set_project("json-parser")
set_version("1.2.2")

if is_mode("release") then
    set_optimize("faster")
    set_strip("all")
elseif is_mode("debug") then
    set_symbols("debug")
    set_optimize("none")
end

set_languages("c99")
set_warnings("all")

target("json-parser")
    set_kind("$(kind)")
    add_files("rbtree.c", "json_parser.c")

target("test_speed")
    set_kind("binary")
    add_files("test_speed.c")
    add_deps("json-parser")

target("parse_json")
    set_kind("binary")
    add_files("test.c")
    add_deps("json-parser")
