project(
	'ocgadget_app',
	'cpp',
	version: '0.1.0',
	default_options: [
		'cpp_std=c++17'
	]
)

cc = meson.get_compiler('cpp')

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
	dependency('ocgadget_msgprotocol'),
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
		dependency('AVFoundation'),
		dependency('CoreMedia')
	]
elif host_machine.system() == 'windows'
endif

srcs = [
	'src/main.cpp',
	'src/ocgadget.cpp',
	'src/msg/msg.cpp',
	'src/ui/ui.cpp',
	'src/ui/gl.cpp',
	'src/capture/capture.cpp',
	'src/capture/test_cap_src.cpp',
	'src/util/image.cpp'
]

if host_machine.system() == 'darwin'
	srcs += [
		'src/ocgadget_macos.cpp',
		'src/ui/metal.mm',
		'src/capture/capture_macos.mm',
		'src/capture/avfoundation.mm',
		'src/msg/msg_macos.cpp'
	]
elif host_machine.system() == 'linux'
	srcs += [
		'src/ocgadget_linux.cpp',
		'src/capture/capture_linux.cpp',
		'src/capture/v4l2.cpp',
		'src/msg/msg_linux.cpp'
	]
elif host_machine.system() == 'windows'
	srcs += [
		'src/ocgadget_windows.cpp',
		'src/capture/capture_windows.cpp',
		'src/capture/mediafoundation.cpp',
		'src/msg/msg_windows.cpp'
	]
endif

exe = executable(
	'ocgadget',
	srcs,
	dependencies: deps
)
