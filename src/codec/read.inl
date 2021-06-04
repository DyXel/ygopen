template<typename T>
constexpr auto read(const uint8_t*& ptr) -> T
{
	T value{};
	std::memcpy(&value, ptr, sizeof(T));
	ptr += sizeof(T);
	return value;
}

template<typename T>
constexpr auto read(uint8_t*& ptr) -> T
{
	return read<T>(const_cast<const uint8_t*&>(ptr));
}
