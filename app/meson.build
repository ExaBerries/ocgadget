project(
	'exaocbot',
	'cpp',
	default_options: [
		'cpp_std=c++20'
	]
)

imgui = subproject('imgui')
imgui_dep = imgui.get_variable('imgui_dep')

exe = executable(
	'exaocbot',
	[
		'src/main.cpp',
		'src/v4l2.cpp',
		'src/ui.cpp',
		'src/util/image.cpp'
	],
	dependencies: [
		imgui_dep,
		dependency('glfw3'),
		dependency('gl'),
		dependency('glew'),
		dependency('threads'),
		dependency('libpng')
	]
)
