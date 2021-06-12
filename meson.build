project('glautomata', 'cpp', version : '1.0')

# Print relevant options.
message('C++ Version = ' + get_option('cpp_std'))
message('Warning level = ' + get_option('warning_level'))
message('Build type = ' + get_option('buildtype'))

# Dependencies.
glfw_dep = dependency('glfw3')
glew_dep = dependency('glew', fallback : ['glew', 'glew_dep'])
opengl_dep = dependency('opengl')
glm_dep = dependency('glm', fallback : ['glm', 'glm_dep'])

deps = [glfw_dep, glew_dep, opengl_dep, glm_dep]

src_file = 'glautomata.cpp'

executable(
    'glautomata',
    sources : src_file,
    dependencies : deps
)