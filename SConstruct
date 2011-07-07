import os

## parameters
ario_src_dir = ARGUMENTS.get("ARIO_SRC_DIR", "../..")
locale_dir = ARGUMENTS.get("LOCALE_DIR", "/usr/local/lib/locale")
destdir = ARGUMENTS.get("DESTDIR", "/usr/local/lib/ario/plugins")
ario_subdirs = ["", "src", "src/plugins", "src/shell", "src/sources"]
libs = ["gtkglext-1.0", "glew", "glut"]

## build
translate = Builder(action = "intltool-merge " + ario_src_dir + "/po " +
                             "$SOURCE $TARGET -d -u -c " + ario_src_dir + 
                             "/po/.intltool-merge-cache")
env = Environment(BUILDERS = {'Translate': translate})

cflags = ""
for subdir in ario_subdirs:
    cflags += "-I" + ario_src_dir + "/" + subdir + " "

cflags += "-DLOCALE_DIR=\\\"\"" + locale_dir + "\"\\\" -DHAVE_CONFIG -g -DDEBUG"


for lib in libs:
    env.ParseConfig("pkg-config " + lib + " --cflags --libs")

lib_target = "coverflow"
lib_sources = ["ario-coverflow-plugin.c", "ario-coverflow.c"]

libcoverflow = env.SharedLibrary(target = lib_target, source = lib_sources, 
                                 CFLAGS=cflags)

plugin_file = env.Translate(source = "coverflow.ario-plugin.desktop.in",
                            target = "coverflow.ario-plugin")

## install
env.Alias(target="install", source=env.Install(destdir, libcoverflow))
env.Alias(target="install", source=env.Install(destdir, plugin_file))
