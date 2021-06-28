project(
	'exaocbot_hubfirmware',
	['cpp', 'c'],
	default_options: [
		'buildtype=minsize',
		'cpp_std=c++20',
		'cpp_rtti=false',
		'cpp_eh=none',
		'b_lto=true',
		'b_staticpic=false'
	]
)

assert(meson.is_cross_build(), 'must be cross built for aduino')

ardu_src = meson.get_cross_property('ardu_src')
ardulib = static_library('ardu', ardu_src)

exe = executable(
	'hubfirmware',
	[
		'src/hubfirmware.cpp'
	],
	dependencies: [
		dependency('exaocbot_msgprotocol')
	],
	link_with: ardulib,
	link_args: '-lm'
)

objcopy = find_program('avr-objcopy')

eep = custom_target(
	'hubfirmware.eep',
	input: exe,
	output: 'hubfirmware.eep',
	command: [
		objcopy, '-O', 'ihex', '-j', '.eeprom',
		'--set-section-flags=.eeprom,=alloc,load',
		'--no-change-warnings', '--change-section-lma',
		'.eeprom=0', '@INPUT@', '@OUTPUT@'
	]
)

hex = custom_target(
	'hubfirmware.hex',
	input: exe,
	output: 'hubfirmware.hex',
	command: [
		objcopy, '-O', 'ihex', '-R', '.eeprom',
		'@INPUT@', '@OUTPUT@'
	],
	depends: eep
)
