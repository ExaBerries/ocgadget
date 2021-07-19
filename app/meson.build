project(
	'exaocbot_app',
	'cpp',
	version: '0.1.0',
	default_options: [
		'cpp_std=c++17'
	]
)

message(host_machine.system())

if host_machine.system() == 'darwin'
	add_languages(['objc', 'objcpp'])
	add_global_arguments([
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
		dependency('QuartzCore'),
		dependency('AVFoundation')
	]
endif

srcs = [
	'src/main.cpp',
	'src/exaocbot.cpp',
	'src/msg/msg.cpp',
	'src/ui/ui.cpp',
	'src/ui/gl.cpp',
	'src/capture/capture.cpp',
	'src/util/image.cpp'
]

if host_machine.system() == 'darwin'
	srcs += [
		'src/exaocbot_macos.cpp',
		'src/ui/metal.mm',
		'src/capture/capture_macos.mm',
		'src/capture/avfoundation.mm',
		'src/msg/msg_macos.cpp'
	]
elif host_machine.system() == 'linux'
	srcs += [
		'src/exaocbot_linux.cpp',
		'src/capture/capture_linux.cpp',
		'src/capture/v4l2.cpp',
		'src/msg/msg_linux.cpp'
	]
endif

exe = executable(
	'exaocbot',
	srcs,
	dependencies: deps
)
