#ifndef YGOPEN_DETAIL_CONFIG_HPP
#define YGOPEN_DETAIL_CONFIG_HPP

#ifdef _MSC_VER
#define YGOPEN_UNREACHABLE() __assume(0)
#else
#define YGOPEN_UNREACHABLE() __builtin_unreachable()
#endif // _MSC_VER

#endif // YGOPEN_DETAIL_CONFIG_HPP
