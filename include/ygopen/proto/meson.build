protoc = find_program('protoc')
protoc_gen = generator(protoc,
	output : ['@BASENAME@.pb.cc', '@BASENAME@.pb.h'],
	arguments : [
		'--proto_path=@CURRENT_SOURCE_DIR@',
		'--cpp_out=@0@:@BUILD_DIR@'.format(pb_optimize_for),
		'@INPUT@',
	]
)

ygopen_proto_src = files(
	'banlist.proto',
	'client_msg.proto',
	'deck.proto',
	'duel_answer.proto',
	'duel_data.proto',
	'duel_msg.proto',
	'replay.proto',
	'room.proto',
	'server_msg.proto',
	'user.proto',
)

generated_proto = protoc_gen.process(ygopen_proto_src)
