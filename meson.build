project('ygopen', 'cpp', license : 'AGPL-3.0-or-later', default_options : 'cpp_std=c++17')

gtest_dep = dependency('gtest', main : true, required : false)
protobuf_dep = dependency('protobuf-lite', version : '>=3.8.0')
# sqlite3_dep = dependency('sqlite3')

ygopen_inc = include_directories('include/')

ygopen_src = files([
	'src/codec/edo9300_ocgcore_decode.cpp',
	'src/codec/edo9300_ocgcore_encode.cpp'
])

subdir('include/ygopen/proto')

ygopen_lib = static_library('ygopen', [ygopen_src, generated_proto], dependencies : protobuf_dep, include_directories : ygopen_inc)

ygopen_dep = declare_dependency(dependencies : protobuf_dep, include_directories : [ygopen_inc, ygopen_lib.private_dir_include()], link_with : ygopen_lib)
meson.override_dependency('ygopen', ygopen_dep)

ygopen_unit_test_src = files([
	'test/frame.cpp',
	'test/place.cpp',
	'test/sequential_wrapper.cpp',
	'test/value_wrappers.cpp'
])

test('unit tests', executable('ut_exe', ygopen_unit_test_src, dependencies : [gtest_dep, ygopen_dep]))
