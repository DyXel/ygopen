protoc = find_program('protoc')
protoc_gen = generator(protoc, output : ['@BASENAME@.pb.cc', '@BASENAME@.pb.h'], arguments : ['--proto_path=@CURRENT_SOURCE_DIR@', '--cpp_out=@BUILD_DIR@', '@INPUT@'])

ygopen_proto_src = files([
	'duel_answer.proto',
	'duel_data.proto',
	'duel_msg.proto',
	'replay.proto'
])

generated_proto = protoc_gen.process(ygopen_proto_src)
