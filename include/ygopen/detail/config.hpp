/*
 * Copyright (c) 2024, Dylam De La Torre <dyxel04@gmail.com>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#ifndef YGOPEN_DETAIL_CONFIG_HPP
#define YGOPEN_DETAIL_CONFIG_HPP

#ifdef _MSC_VER
#define YGOPEN_UNREACHABLE() __assume(0)
#else
#define YGOPEN_UNREACHABLE() __builtin_unreachable()
#endif // _MSC_VER

#if !defined(YGOPEN_HAS_CONCEPTS)
#if !defined(YGOPEN_DISABLE_CONCEPTS)
#if defined(__cpp_concepts)
#define YGOPEN_HAS_CONCEPTS 1
#define YGOPEN_CONCEPT(x) x x
#else
#define YGOPEN_CONCEPT(x) typename x
#endif // defined(__cpp_concepts)
#endif // !defined(YGOPEN_DISABLE_CONCEPTS)
#endif // !defined(YGOPEN_HAS_CONCEPTS)

#define YGOPEN_TYPEOF(x) \
	typename std::remove_cv_t<std::remove_reference_t<decltype(x)>>

#endif // YGOPEN_DETAIL_CONFIG_HPP
