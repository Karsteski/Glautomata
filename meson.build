project('glautomata', 'cpp', version : '1.0')

# Print relevant options.
message('C++ Version = ' + get_option('cpp_std'))
message('Warning level = ' + get_option('warning_level'))
message('Build type = ' + get_option('buildtype'))

# Dependencies.
sdl2_dep = dependency('sdl2', fallback: ['sdl2', 'sdl2_dep'])
glew_dep = dependency('glew', fallback : ['glew', 'glew_dep'])
opengl_dep = dependency('opengl')

deps = [sdl2_dep, glew_dep, opengl_dep]

src_file = 'glautomata.cpp'

executable(
    'glautomata',
    sources : src_file,
    dependencies : deps
)