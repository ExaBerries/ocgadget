project(
	'exaocbot_app',
	'cpp',
	version: '0.1.0',
	default_options: [
		'cpp_std=c++20'
	]
)

message(host_machine.system())

if host_machine.system() == 'darwin'
	add_languages(['objc','objcpp'])
	add_global_arguments([
		'-std=c++2a',
		'-fobjc-arc'
	], language: 'objcpp')
endif

imgui = subproject('imgui')
imgui_dep = imgui.get_variable('imgui_dep')

deps = [
	dependency('exaocbot_msgprotocol'),
	imgui_dep,
	dependency('glfw3'),
	dependency('glew'),
	dependency('gl'),
	dependency('pngpp'),
	dependency('threads')
]

if host_machine.system() == 'darwin'
	deps += [
		dependency('Metal'),
		dependency('MetalKit'),
		dependency('QuartzCore')
	]
endif

srcs = [
	'src/main.cpp',
	'src/v4l2.cpp',
	'src/ui/ui.cpp',
	'src/ui/gl.cpp',
	'src/capture/capture.cpp',
	'src/util/image.cpp'
]

if host_machine.system() == 'darwin'
	srcs += [
		'src/ui/metal.mm'
	]
endif

exe = executable(
	'exaocbot',
	srcs,
	dependencies: deps
)
