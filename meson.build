project('glautomata', 'cpp', version : '1.0')

# Print relevant options.
message('C++ Version = ' + get_option('cpp_std'))
message('Warning level = ' + get_option('warning_level'))
message('Build type = ' + get_option('buildtype'))


# GLFW dependency using CMake
cmake = import('cmake')
## Set CMake options for building GLFW
glfw_opt_var = cmake.subproject_options()
glfw_opt_var.add_cmake_defines({'BUILD_SHARED_LIBS': true})
glfw_opt_var.add_cmake_defines({'GLFW_BUILD_EXAMPLES': false})
glfw_opt_var.add_cmake_defines({'GLFW_BUILD_TESTS': false})
glfw_opt_var.add_cmake_defines({'GLFW_BUILD_DOCS': false})
# Configure the CMake project
glfw_sub_proj = cmake.subproject('glfw', options: glfw_opt_var)
# Fetch dependency object
glfw_dep = glfw_sub_proj.dependency('glfw')


# Dependencies
glew_dep = dependency('glew', fallback : ['glew', 'glew_dep'])
opengl_dep = dependency('opengl')
glm_dep = dependency('glm', fallback : ['glm', 'glm_dep'])

deps = [glfw_dep, glew_dep, opengl_dep, glm_dep]

src_file = 'src/glautomata.cpp'

executable(
    'glautomata',
    sources : src_file,
    dependencies : deps
)