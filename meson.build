project('ygopen', 'cpp',
	license : 'AGPL-3.0-or-later',
	default_options : 'cpp_std=c++17'
)

unittests_opt       = get_option('unittests')
pb_optimize_for_opt = get_option('pb_optimize_for')

gtest_dep    = dependency('gtest', main : true, required : unittests_opt)
protobuf_dep = dependency(
	pb_optimize_for_opt == 'lite' ? 'protobuf-lite' : 'protobuf',
	version : '>=3.8.0',
)

ygopen_inc = include_directories('include/')

ygopen_src = files(
	'src/codec/edo9300_ocgcore_decode.cpp',
	'src/codec/edo9300_ocgcore_encode.cpp'
)

subdir('include/ygopen/proto')

ygopen_lib = static_library('ygopen', [ygopen_src, generated_proto],
	dependencies : protobuf_dep,
	include_directories : ygopen_inc
)

ygopen_dep = declare_dependency(
	dependencies : protobuf_dep,
	include_directories : [ygopen_inc, ygopen_lib.private_dir_include()],
	link_with : ygopen_lib
)
meson.override_dependency('ygopen', ygopen_dep)

if unittests_opt.require(gtest_dep.found()).allowed()
	ygopen_unit_tests_src = files(
		'test/banlist.cpp',
		'test/bit.cpp',
		'test/board.cpp',
		'test/deck.cpp',
		'test/frame.cpp',
		'test/parse_event.cpp',
		'test/place.cpp',
		'test/room.cpp',
		'test/undoable.cpp',
	)

	ygopen_unit_tests_exe = executable('ygopen_ut', ygopen_unit_tests_src,
		dependencies : [gtest_dep, ygopen_dep]
	)

	test('ygopen unit tests', ygopen_unit_tests_exe)
endif
