//
// SqratGlobalMethods: Global Methods
//

//
// Copyright (c) 2009 Brandon Jones
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
//  1. The origin of this software must not be misrepresented; you must not
//  claim that you wrote the original software. If you use this software
//  in a product, an acknowledgment in the product documentation would be
//  appreciated but is not required.
//
//  2. Altered source versions must be plainly marked as such, and must not be
//  misrepresented as being the original software.
//
//  3. This notice may not be removed or altered from any source
//  distribution.
//

#if !defined(_SQRAT_OVERLOAD_METHODS_H_)
#define _SQRAT_OVERLOAD_METHODS_H_

#include <typeinfo>
#include <squirrel.h>
#include <sqstdaux.h>
#include "sqratTypes.h"
#include "sqratUtil.h"
#include "sqratGlobalMethods.h"
#include "sqratMemberMethods.h"

namespace Sqrat
{
//
// Overload parameter name generating functions
//

// Default parameter name template
template<class T>
struct SqOverloadParameter {
    static string Name() {
        if (is_reference<T>::value) {
#ifdef SQUNICODE
            return string_to_wstring(typeid(typename remove_reference<typename remove_const<T>::type>::type).name());
#else
            return typeid(typename remove_reference<typename remove_const<T>::type>::type).name();
#endif
        } else {
#ifdef SQUNICODE
            return string_to_wstring(typeid(typename remove_pointer<typename remove_const<T>::type>::type).name());
#else
            return typeid(typename remove_pointer<typename remove_const<T>::type>::type).name();
#endif
        }
    }
};

// Integer parameter name specializations
#define SCRAT_OVERLOAD_NAME_INTEGER( type ) \
    template<> \
    struct SqOverloadParameter<type> { \
        static string Name() { \
            return _SC("i"); \
        } \
    }; \
    \
    template<> \
    struct SqOverloadParameter<const type> { \
        static string Name() { \
            return _SC("i"); \
        } \
    }; \
    \
    template<> \
    struct SqOverloadParameter<const type&> { \
        static string Name() { \
            return _SC("i"); \
        } \
    };

SCRAT_OVERLOAD_NAME_INTEGER(unsigned int)
SCRAT_OVERLOAD_NAME_INTEGER(signed int)
SCRAT_OVERLOAD_NAME_INTEGER(unsigned long)
SCRAT_OVERLOAD_NAME_INTEGER(signed long)
SCRAT_OVERLOAD_NAME_INTEGER(unsigned short)
SCRAT_OVERLOAD_NAME_INTEGER(signed short)
SCRAT_OVERLOAD_NAME_INTEGER(unsigned char)
SCRAT_OVERLOAD_NAME_INTEGER(signed char)
SCRAT_OVERLOAD_NAME_INTEGER(unsigned long long)
SCRAT_OVERLOAD_NAME_INTEGER(signed long long)

#ifdef _MSC_VER
#if defined(__int64)
SCRAT_OVERLOAD_NAME_INTEGER(unsigned __int64)
SCRAT_OVERLOAD_NAME_INTEGER(signed __int64)
#endif
#endif

// Float parameter name specializations
#define SCRAT_OVERLOAD_NAME_FLOAT( type ) \
    template<> \
    struct SqOverloadParameter<type> { \
        static string Name() { \
            return _SC("f"); \
        } \
    }; \
    \
    template<> \
    struct SqOverloadParameter<const type> { \
        static string Name() { \
            return _SC("f"); \
        } \
    }; \
    \
    template<> \
    struct SqOverloadParameter<const type&> { \
        static string Name() { \
            return _SC("f"); \
        } \
    };

SCRAT_OVERLOAD_NAME_FLOAT(float)
SCRAT_OVERLOAD_NAME_FLOAT(double)

// Boolean parameter name specializations
template<>
struct SqOverloadParameter<bool> {
    static string Name() {
        return _SC("b");
    }
};

template<>
struct SqOverloadParameter<const bool> {
    static string Name() {
        return _SC("b");
    }
};

template<>
struct SqOverloadParameter<const bool&> {
    static string Name() {
        return _SC("b");
    }
};

// String parameter name specializations
template<>
struct SqOverloadParameter<SQChar*> {
    static string Name() {
        return _SC("s");
    }
};

template<>
struct SqOverloadParameter<const SQChar*> {
    static string Name() {
        return _SC("s");
    }
};

template<>
struct SqOverloadParameter<string> {
    static string Name() {
        return _SC("s");
    }
};

template<>
struct SqOverloadParameter<const string&> {
    static string Name() {
        return _SC("s");
    }
};

#ifdef SQUNICODE
template<>
struct SqOverloadParameter<char*> {
    static string Name() {
        return _SC("s");
    }
};

template<>
struct SqOverloadParameter<const char*> {
    static string Name() {
        return _SC("s");
    }
};

template<>
struct SqOverloadParameter<std::string> {
    string Name() {
        return _SC("s");
    }
};

template<>
struct SqOverloadParameter<const std::string&> {
    string Name() {
        return _SC("s");
    }
};
#endif

// Array parameter name specialization
class Array;
template<>
struct SqOverloadParameter<Array> {
    string Name() {
        return _SC("a");
    }
};

// Function parameter name specialization
class Function;
template<>
struct SqOverloadParameter<Function> {
    string Name() {
        return _SC("c");
    }
};

// Table parameter name specialization
class Table;
template<>
struct SqOverloadParameter<Table> {
    string Name() {
        return _SC("t");
    }
};

//
// Gets an overload function from Squirrel arguments
//
void GetOverload(HSQUIRRELVM vm, const SQChar* funcName, int argCount)
{
    sq_pushstring(vm, _SC("__sqrat_ol"), -1);
    if (SQ_FAILED(sq_get(vm, 1))) {
        return sq_throwerror(vm, _SC("the index '__sqrat_ol' does not exist"));
    }

    sq_pushstring(vm, funcName, -1);
    if (SQ_FAILED(sq_get(vm, -2))) {
        return sq_throwerror(vm, (string("the index '") + string(funcName) + string("' does not exist")).c_str());
    }

    sq_pushinteger(vm, argCount);
    if (SQ_FAILED(sq_get(vm, -2))) {
        return sq_throwerror(vm, _SC("wrong number of parameters"));
    }

    bool typeError = false;
    int i;
    for (i = 2; i <= argCount + 1; ++i) {
        if (typeError) break;
        switch (sq_gettype(vm, i)) {
            case OT_INSTANCE:
                ClassTypeDataBase* instanceType;
                sq_gettypetag(vm, i, (SQUserPointer*)&instanceType);
                if (instanceType == NULL) {
                    SQInteger top = sq_gettop(vm);
                    sq_getclass(vm, i);
                    while (instanceType == NULL) {
                        sq_getbase(vm, -1);
                        if (sq_gettype(vm, -1) == OT_NULL) {
                            typeError = true;
                            break;
                        }
                        sq_gettypetag(vm, -1, (SQUserPointer*)&instanceType);
                    }
                    sq_settop(vm, top);
                    if (typeError) break;
                }
                sq_pushstring(vm, instanceType->typeName, -1);
                while (SQ_FAILED(sq_get(vm, -2))) {
                    instanceType = instanceType->baseClass;
                    if (instanceType == NULL) {
                        typeError = true;
                        break;
                    }
                    sq_pushstring(vm, instanceType->typeName, -1);
                }
                break;
            case OT_BOOL:
                sq_pushstring(vm, SqOverloadParameter<bool>.Name(), -1);
                if (SQ_FAILED(sq_get(vm, -2))) {
                    sq_pushstring(vm, SqOverloadParameter<SQInteger>.Name(), -1);
                    if (SQ_FAILED(sq_get(vm, -2))) {
                        sq_pushstring(vm, SqOverloadParameter<SQFloat>.Name(), -1);
                        if (SQ_FAILED(sq_get(vm, -2))) {
                            typeError = true;
                        }
                    }
                }
                break;
            case OT_INTEGER:
                sq_pushstring(vm, SqOverloadParameter<SQInteger>.Name(), -1);
                if (SQ_FAILED(sq_get(vm, -2))) {
                    sq_pushstring(vm, SqOverloadParameter<SQFloat>.Name(), -1);
                    if (SQ_FAILED(sq_get(vm, -2))) {
                        sq_pushstring(vm, SqOverloadParameter<bool>.Name(), -1);
                        if (SQ_FAILED(sq_get(vm, -2))) {
                            typeError = true;
                        }
                    }
                }
                break;
            case OT_FLOAT:
                sq_pushstring(vm, SqOverloadParameter<SQFloat>.Name(), -1);
                if (SQ_FAILED(sq_get(vm, -2))) {
                    sq_pushstring(vm, SqOverloadParameter<SQInteger>.Name(), -1);
                    if (SQ_FAILED(sq_get(vm, -2))) {
                        sq_pushstring(vm, SqOverloadParameter<bool>.Name(), -1);
                        if (SQ_FAILED(sq_get(vm, -2))) {
                            typeError = true;
                        }
                    }
                }
                break;
            case OT_STRING:
                sq_pushstring(vm, SqOverloadParameter<SQChar*>.Name(), -1);
                if (SQ_FAILED(sq_get(vm, -2))) {
                    typeError = true;
                }
                break;
            case OT_ARRAY:
                sq_pushstring(vm, SqOverloadParameter<Array>.Name(), -1);
                if (SQ_FAILED(sq_get(vm, -2))) {
                    typeError = true;
                }
                break;
            case OT_CLOSURE:
                sq_pushstring(vm, SqOverloadParameter<Function>.Name(), -1);
                if (SQ_FAILED(sq_get(vm, -2))) {
                    typeError = true;
                }
                break;
            case OT_NATIVECLOSURE:
                sq_pushstring(vm, SqOverloadParameter<Function>.Name(), -1);
                if (SQ_FAILED(sq_get(vm, -2))) {
                    typeError = true;
                }
                break;
            case OT_TABLE:
                sq_pushstring(vm, SqOverloadParameter<Table>.Name(), -1);
                if (SQ_FAILED(sq_get(vm, -2))) {
                    typeError = true;
                }
                break;
            default:
                sq_pushstring(vm, SqOverloadParameter<bool>.Name(), -1);
                if (SQ_FAILED(sq_get(vm, -2))) {
                    sq_pushstring(vm, SqOverloadParameter<SQChar*>.Name(), -1);
                    if (SQ_FAILED(sq_get(vm, -2))) {
                        typeError = true;
                    }
                }
                break;
        }
    }

    if (typeError) {
        string expectedTypes;
        sq_pushnull(vm);
        while (SQ_SUCCEEDED(sq_next(vm, -2))) {
            sq_pop(vm, 1);
            const SQChar* parameterName;
            sq_tostring(vm, -1);
            sq_getstring(vm, -1, &parameterName);
            if (parameterName == SqOverloadParameter<bool>.Name()) {
                expectedTypes += _SC("bool|");
            } else if (parameterName == SqOverloadParameter<SQInteger>.Name()) {
                expectedTypes += _SC("integer|");
            } else if (parameterName == SqOverloadParameter<SQFloat>.Name()) {
                expectedTypes += _SC("float|");
            } else if (parameterName == SqOverloadParameter<SQChar*>.Name()) {
                expectedTypes += _SC("string|");
            } else if (parameterName == SqOverloadParameter<Array>.Name()) {
                expectedTypes += _SC("array|");
            } else if (parameterName == SqOverloadParameter<Function>.Name()) {
                expectedTypes += _SC("closure|");
            } else if (parameterName == SqOverloadParameter<Table>.Name()) {
                expectedTypes += _SC("table|");
            } else {
                ClassTypeDataBase* expectedInstanceType = ClassTypeMap::GetByTypeName(vm, parameterName);
                if (expectedInstanceType != NULL) {
                    expectedTypes += expectedInstanceType->className + _SC("|");
                } else {
                    expectedTypes += _SC("unknown|");
                }
            }
            sq_pop(vm, 2);
        }
        sq_pop(vm, 1);
        if (expectedTypes.size() > 0) {
            expectedTypes.resize(expectedTypes.size() - 1);
        }
        Error::Instance().Throw(vm, Error::FormatTypeError(vm, i, expectedTypes));
    }
}

//
// Squirrel Overload Functions
//
template <class R>
class SqOverload
{
public:
    static SQInteger Func(HSQUIRRELVM vm) {
        // Get the arg count
        int argCount = sq_gettop(vm) - 2;

        const SQChar* funcName;
        sq_getstring(vm, -1, &funcName); // get the function name (free variable)

        GetOverload(vm, funcName, argCount);
        if (Error::Instance().Occurred(vm)) {
            return sq_throwerror(vm, Error::Instance().Message(vm).c_str());
        }

        // Push the args again
        for (int i = 1; i <= argCount + 1; ++i) {
            sq_push(vm, i);
        }

        sq_call(vm, argCount + 1, true, ErrorHandling::IsEnabled());
        if (Error::Instance().Occurred(vm)) {
            return sq_throwerror(vm, Error::Instance().Message(vm).c_str());
        }

        return 1;
    }
};

//
// void return specialization
//
template <>
class SqOverload<void>
{
public:
    static SQInteger Func(HSQUIRRELVM vm) {
        // Get the arg count
        int argCount = sq_gettop(vm) - 2;

        const SQChar* funcName;
        sq_getstring(vm, -1, &funcName); // get the function name (free variable)

        GetOverload(vm, funcName, argCount);
        if (Error::Instance().Occurred(vm)) {
            return sq_throwerror(vm, Error::Instance().Message(vm).c_str());
        }

        // Push the args again
        for (int i = 1; i <= argCount + 1; ++i) {
            sq_push(vm, i);
        }

        sq_call(vm, argCount + 1, false, ErrorHandling::IsEnabled());
        if (Error::Instance().Occurred(vm)) {
            return sq_throwerror(vm, Error::Instance().Message(vm).c_str());
        }

        return 0;
    }
};

//
// Global Overloaded Function Resolvers
//

// Arg Count 0
template <class R>
SQFUNCTION SqGlobalOverloadedFunc(R (*method)())
{
    return &SqGlobal<R>::template Func0<true>;
}

template <class R>
SQFUNCTION SqGlobalOverloadedFunc(R & (*method)())
{
    return &SqGlobal<R&>::template Func0<true>;
}

// Arg Count 1
template <class R, class A1>
SQFUNCTION SqGlobalOverloadedFunc(R (*method)(A1))
{
    return &SqGlobal<R>::template Func1<A1, 2, true>;
}

template <class R, class A1>
SQFUNCTION SqGlobalOverloadedFunc(R & (*method)(A1))
{
    return &SqGlobal<R&>::template Func1<A1, 2, true>;
}

// Arg Count 2
template <class R, class A1, class A2>
SQFUNCTION SqGlobalOverloadedFunc(R (*method)(A1, A2))
{
    return &SqGlobal<R>::template Func2<A1, A2, 2, true>;
}

template <class R, class A1, class A2>
SQFUNCTION SqGlobalOverloadedFunc(R & (*method)(A1, A2))
{
    return &SqGlobal<R&>::template Func2<A1, A2, 2, true>;
}

// Arg Count 3
template <class R, class A1, class A2, class A3>
SQFUNCTION SqGlobalOverloadedFunc(R (*method)(A1, A2, A3))
{
    return &SqGlobal<R>::template Func3<A1, A2, A3, 2, true>;
}

template <class R, class A1, class A2, class A3>
SQFUNCTION SqGlobalOverloadedFunc(R & (*method)(A1, A2, A3))
{
    return &SqGlobal<R&>::template Func3<A1, A2, A3, 2, true>;
}

// Arg Count 4
template <class R, class A1, class A2, class A3, class A4>
SQFUNCTION SqGlobalOverloadedFunc(R (*method)(A1, A2, A3, A4))
{
    return &SqGlobal<R>::template Func4<A1, A2, A3, A4, 2, true>;
}

template <class R, class A1, class A2, class A3, class A4>
SQFUNCTION SqGlobalOverloadedFunc(R & (*method)(A1, A2, A3, A4))
{
    return &SqGlobal<R& >::template Func4<A1, A2, A3, A4, 2, true>;
}

// Arg Count 5
template <class R, class A1, class A2, class A3, class A4, class A5>
SQFUNCTION SqGlobalOverloadedFunc(R (*method)(A1, A2, A3, A4, A5))
{
    return &SqGlobal<R>::template Func5<A1, A2, A3, A4, A5, 2, true>;
}

template <class R, class A1, class A2, class A3, class A4, class A5>
SQFUNCTION SqGlobalOverloadedFunc(R & (*method)(A1, A2, A3, A4, A5))
{
    return &SqGlobal<R&>::template Func5<A1, A2, A3, A4, A5, 2, true>;
}

// Arg Count 6
template <class R, class A1, class A2, class A3, class A4, class A5, class A6>
SQFUNCTION SqGlobalOverloadedFunc(R (*method)(A1, A2, A3, A4, A5, A6))
{
    return &SqGlobal<R>::template Func6<A1, A2, A3, A4, A5, A6, 2, true>;
}

template <class R, class A1, class A2, class A3, class A4, class A5, class A6>
SQFUNCTION SqGlobalOverloadedFunc(R & (*method)(A1, A2, A3, A4, A5, A6))
{
    return &SqGlobal<R&>::template Func6<A1, A2, A3, A4, A5, A6, 2, true>;
}

// Arg Count 7
template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
SQFUNCTION SqGlobalOverloadedFunc(R (*method)(A1, A2, A3, A4, A5, A6, A7))
{
    return &SqGlobal<R>::template Func7<A1, A2, A3, A4, A5, A6, A7, 2, true>;
}

template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
SQFUNCTION SqGlobalOverloadedFunc(R & (*method)(A1, A2, A3, A4, A5, A6, A7))
{
    return &SqGlobal<R&>::template Func7<A1, A2, A3, A4, A5, A6, A7, 2, true>;
}

// Arg Count 8
template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
SQFUNCTION SqGlobalOverloadedFunc(R (*method)(A1, A2, A3, A4, A5, A6, A7, A8))
{
    return &SqGlobal<R>::template Func8<A1, A2, A3, A4, A5, A6, A7, A8, 2, true>;
}

template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
SQFUNCTION SqGlobalOverloadedFunc(R & (*method)(A1, A2, A3, A4, A5, A6, A7, A8))
{
    return &SqGlobal<R&>::template Func8<A1, A2, A3, A4, A5, A6, A7, A8, 2, true>;
}

// Arg Count 9
template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
SQFUNCTION SqGlobalOverloadedFunc(R (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9))
{
    return &SqGlobal<R>::template Func9<A1, A2, A3, A4, A5, A6, A7, A8, A9, 2, true>;
}

template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
SQFUNCTION SqGlobalOverloadedFunc(R & (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9))
{
    return &SqGlobal<R&>::template Func9<A1, A2, A3, A4, A5, A6, A7, A8, A9, 2, true>;
}

// Arg Count 10
template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
SQFUNCTION SqGlobalOverloadedFunc(R (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10))
{
    return &SqGlobal<R>::template Func10<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, 2, true>;
}

template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
SQFUNCTION SqGlobalOverloadedFunc(R & (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10))
{
    return &SqGlobal<R&>::template Func10<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, 2, true>;
}

// Arg Count 11
template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
SQFUNCTION SqGlobalOverloadedFunc(R (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11))
{
    return &SqGlobal<R>::template Func11<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, 2, true>;
}

template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
SQFUNCTION SqGlobalOverloadedFunc(R & (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11))
{
    return &SqGlobal<R&>::template Func11<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, 2, true>;
}

// Arg Count 12
template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
SQFUNCTION SqGlobalOverloadedFunc(R (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12))
{
    return &SqGlobal<R>::template Func12<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, 2, true>;
}

template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
SQFUNCTION SqGlobalOverloadedFunc(R & (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12))
{
    return &SqGlobal<R&>::template Func12<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, 2, true>;
}

// Arg Count 13
template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13>
SQFUNCTION SqGlobalOverloadedFunc(R (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13))
{
    return &SqGlobal<R>::template Func13<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, 2, true>;
}

template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13>
SQFUNCTION SqGlobalOverloadedFunc(R & (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13))
{
    return &SqGlobal<R&>::template Func13<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, 2, true>;
}

// Arg Count 14
template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13, class A14>
SQFUNCTION SqGlobalOverloadedFunc(R (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14))
{
    return &SqGlobal<R>::template Func14<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, 2, true>;
}
template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13, class A14>
SQFUNCTION SqGlobalOverloadedFunc(R & (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14))
{
    return &SqGlobal<R&>::template Func14<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, 2, true>;
}

//
// Member Global Overloaded Function Resolvers
//

// Arg Count 1
template <class R, class A1>
SQFUNCTION SqMemberGlobalOverloadedFunc(R (*method)(A1))
{
    return &SqGlobal<R>::template Func1<A1, 1, true>;
}

template <class R, class A1>
SQFUNCTION SqMemberGlobalOverloadedFunc(R & (*method)(A1))
{
    return &SqGlobal<R&>::template Func1<A1, 1, true>;
}

// Arg Count 2
template <class R, class A1, class A2>
SQFUNCTION SqMemberGlobalOverloadedFunc(R (*method)(A1, A2))
{
    return &SqGlobal<R>::template Func2<A1, A2, 1, true>;
}

template <class R, class A1, class A2>
SQFUNCTION SqMemberGlobalOverloadedFunc(R & (*method)(A1, A2))
{
    return &SqGlobal<R&>::template Func2<A1, A2, 1, true>;
}

// Arg Count 3
template <class R, class A1, class A2, class A3>
SQFUNCTION SqMemberGlobalOverloadedFunc(R (*method)(A1, A2, A3))
{
    return &SqGlobal<R>::template Func3<A1, A2, A3, 1, true>;
}

template <class R, class A1, class A2, class A3>
SQFUNCTION SqMemberGlobalOverloadedFunc(R & (*method)(A1, A2, A3))
{
    return &SqGlobal<R&>::template Func3<A1, A2, A3, 1, true>;
}

// Arg Count 4
template <class R, class A1, class A2, class A3, class A4>
SQFUNCTION SqMemberGlobalOverloadedFunc(R (*method)(A1, A2, A3, A4))
{
    return &SqGlobal<R>::template Func4<A1, A2, A3, A4, 1, true>;
}

template <class R, class A1, class A2, class A3, class A4>
SQFUNCTION SqMemberGlobalOverloadedFunc(R & (*method)(A1, A2, A3, A4))
{
    return &SqGlobal<R&>::template Func4<A1, A2, A3, A4, 1, true>;
}

// Arg Count 5
template <class R, class A1, class A2, class A3, class A4, class A5>
SQFUNCTION SqMemberGlobalOverloadedFunc(R (*method)(A1, A2, A3, A4, A5))
{
    return &SqGlobal<R>::template Func5<A1, A2, A3, A4, A5, 1, true>;
}

template <class R, class A1, class A2, class A3, class A4, class A5>
SQFUNCTION SqMemberGlobalOverloadedFunc(R & (*method)(A1, A2, A3, A4, A5))
{
    return &SqGlobal<R&>::template Func5<A1, A2, A3, A4, A5, 1, true>;
}

// Arg Count 6
template <class R, class A1, class A2, class A3, class A4, class A5, class A6>
SQFUNCTION SqMemberGlobalOverloadedFunc(R (*method)(A1, A2, A3, A4, A5, A6))
{
    return &SqGlobal<R>::template Func6<A1, A2, A3, A4, A5, A6, 1, true>;
}

template <class R, class A1, class A2, class A3, class A4, class A5, class A6>
SQFUNCTION SqMemberGlobalOverloadedFunc(R & (*method)(A1, A2, A3, A4, A5, A6))
{
    return &SqGlobal<R&>::template Func6<A1, A2, A3, A4, A5, A6, 1, true>;
}

// Arg Count 7
template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
SQFUNCTION SqMemberGlobalOverloadedFunc(R (*method)(A1, A2, A3, A4, A5, A6, A7))
{
    return &SqGlobal<R>::template Func7<A1, A2, A3, A4, A5, A6, A7, 1, true>;
}

template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
SQFUNCTION SqMemberGlobalOverloadedFunc(R & (*method)(A1, A2, A3, A4, A5, A6, A7))
{
    return &SqGlobal<R&>::template Func7<A1, A2, A3, A4, A5, A6, A7, 1, true>;
}

// Arg Count 8
template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
SQFUNCTION SqMemberGlobalOverloadedFunc(R (*method)(A1, A2, A3, A4, A5, A6, A7, A8))
{
    return &SqGlobal<R>::template Func8<A1, A2, A3, A4, A5, A6, A7, A8, 1, true>;
}

template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
SQFUNCTION SqMemberGlobalOverloadedFunc(R & (*method)(A1, A2, A3, A4, A5, A6, A7, A8))
{
    return &SqGlobal<R&>::template Func8<A1, A2, A3, A4, A5, A6, A7, A8, 1, true>;
}

// Arg Count 9
template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
SQFUNCTION SqMemberGlobalOverloadedFunc(R (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9))
{
    return &SqGlobal<R>::template Func9<A1, A2, A3, A4, A5, A6, A7, A8, A9, 1, true>;
}

template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
SQFUNCTION SqMemberGlobalOverloadedFunc(R & (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9))
{
    return &SqGlobal<R&>::template Func9<A1, A2, A3, A4, A5, A6, A7, A8, A9, 1, true>;
}

// Arg Count 10
template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
SQFUNCTION SqMemberGlobalOverloadedFunc(R (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10))
{
    return &SqGlobal<R>::template Func10<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, 1, true>;
}

template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
SQFUNCTION SqMemberGlobalOverloadedFunc(R & (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10))
{
    return &SqGlobal<R&>::template Func10<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, 1, true>;
}

// Arg Count 11
template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
SQFUNCTION SqMemberGlobalOverloadedFunc(R (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11))
{
    return &SqGlobal<R>::template Func11<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, 1, true>;
}

template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
SQFUNCTION SqMemberGlobalOverloadedFunc(R & (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11))
{
    return &SqGlobal<R&>::template Func11<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, 1, true>;
}

// Arg Count 12
template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
SQFUNCTION SqMemberGlobalOverloadedFunc(R (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12))
{
    return &SqGlobal<R>::template Func12<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, 1, true>;
}

template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
SQFUNCTION SqMemberGlobalOverloadedFunc(R & (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12))
{
    return &SqGlobal<R&>::template Func12<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, 1, true>;
}

// Arg Count 13
template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13>
SQFUNCTION SqMemberGlobalOverloadedFunc(R (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13))
{
    return &SqGlobal<R>::template Func13<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, 1, true>;
}

template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13>
SQFUNCTION SqMemberGlobalOverloadedFunc(R & (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13))
{
    return &SqGlobal<R&>::template Func13<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, 1, true>;
}

// Arg Count 14
template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13, class A14>
SQFUNCTION SqMemberGlobalOverloadedFunc(R (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14))
{
    return &SqGlobal<R>::template Func14<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, 1, true>;
}

template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13, class A14>
SQFUNCTION SqMemberGlobalOverloadedFunc(R & (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14))
{
    return &SqGlobal<R&>::template Func14<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, 1, true>;
}

//
// Member Overloaded Function Resolvers
//

// Arg Count 0
template <class C, class R>
inline SQFUNCTION SqMemberOverloadedFunc(R (C::*method)())
{
    return &SqMember<C, R>::template Func0<true>;
}

template <class C, class R>
inline SQFUNCTION SqMemberOverloadedFunc(R (C::*method)() const)
{
    return &SqMember<C, R>::template Func0C<true>;
}

template <class C, class R>
inline SQFUNCTION SqMemberOverloadedFunc(R & (C::*method)())
{
    return &SqMember<C, R&>::template Func0<true>;
}
template <class C, class R>
inline SQFUNCTION SqMemberOverloadedFunc(R & (C::*method)() const)
{
    return &SqMember<C, R&>::template Func0C<true>;
}


// Arg Count 1
template <class C, class R, class A1>
inline SQFUNCTION SqMemberOverloadedFunc(R (C::*method)(A1))
{
    return &SqMember<C, R>::template Func1<A1, true>;
}

template <class C, class R, class A1>
inline SQFUNCTION SqMemberOverloadedFunc(R (C::*method)(A1) const)
{
    return &SqMember<C, R>::template Func1C<A1, true>;
}

template <class C, class R, class A1>
inline SQFUNCTION SqMemberOverloadedFunc(R & (C::*method)(A1))
{
    return &SqMember<C, R&>::template Func1<A1, true>;
}

template <class C, class R, class A1>
inline SQFUNCTION SqMemberOverloadedFunc(R & (C::*method)(A1) const)
{
    return &SqMember<C, R&>::template Func1C<A1, true>;
}

// Arg Count 2
template <class C, class R, class A1, class A2>
inline SQFUNCTION SqMemberOverloadedFunc(R (C::*method)(A1, A2))
{
    return &SqMember<C, R>::template Func2<A1, A2, true>;
}

template <class C, class R, class A1, class A2>
inline SQFUNCTION SqMemberOverloadedFunc(R (C::*method)(A1, A2) const)
{
    return &SqMember<C, R>::template Func2C<A1, A2, true>;
}

template <class C, class R, class A1, class A2>
inline SQFUNCTION SqMemberOverloadedFunc(R & (C::*method)(A1, A2))
{
    return &SqMember<C, R&>::template Func2<A1, A2, true>;
}

template <class C, class R, class A1, class A2>
inline SQFUNCTION SqMemberOverloadedFunc(R & (C::*method)(A1, A2) const)
{
    return &SqMember<C, R&>::template Func2C<A1, A2, true>;
}

// Arg Count 3
template <class C, class R, class A1, class A2, class A3>
inline SQFUNCTION SqMemberOverloadedFunc(R (C::*method)(A1, A2, A3))
{
    return &SqMember<C, R>::template Func3<A1, A2, A3, true>;
}

template <class C, class R, class A1, class A2, class A3>
inline SQFUNCTION SqMemberOverloadedFunc(R (C::*method)(A1, A2, A3) const)
{
    return &SqMember<C, R>::template Func3C<A1, A2, A3, true>;
}
template <class C, class R, class A1, class A2, class A3>
inline SQFUNCTION SqMemberOverloadedFunc(R & (C::*method)(A1, A2, A3))
{
    return &SqMember<C, R&>::template Func3<A1, A2, A3, true>;
}

template <class C, class R, class A1, class A2, class A3>
inline SQFUNCTION SqMemberOverloadedFunc(R & (C::*method)(A1, A2, A3) const)
{
    return &SqMember<C, R&>::template Func3C<A1, A2, A3, true>;
}

// Arg Count 4
template <class C, class R, class A1, class A2, class A3, class A4>
inline SQFUNCTION SqMemberOverloadedFunc(R (C::*method)(A1, A2, A3, A4))
{
    return &SqMember<C, R>::template Func4<A1, A2, A3, A4, true>;
}

template <class C, class R, class A1, class A2, class A3, class A4>
inline SQFUNCTION SqMemberOverloadedFunc(R (C::*method)(A1, A2, A3, A4) const)
{
    return &SqMember<C, R>::template Func4C<A1, A2, A3, A4, true>;
}

template <class C, class R, class A1, class A2, class A3, class A4>
inline SQFUNCTION SqMemberOverloadedFunc(R & (C::*method)(A1, A2, A3, A4))
{
    return &SqMember<C, R&>::template Func4<A1, A2, A3, A4, true>;
}

template <class C, class R, class A1, class A2, class A3, class A4>
inline SQFUNCTION SqMemberOverloadedFunc(R & (C::*method)(A1, A2, A3, A4) const)
{
    return &SqMember<C, R&>::template Func4C<A1, A2, A3, A4, true>;
}

// Arg Count 5
template <class C, class R, class A1, class A2, class A3, class A4, class A5>
inline SQFUNCTION SqMemberOverloadedFunc(R (C::*method)(A1, A2, A3, A4, A5))
{
    return &SqMember<C, R>::template Func5<A1, A2, A3, A4, A5, true>;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5>
inline SQFUNCTION SqMemberOverloadedFunc(R (C::*method)(A1, A2, A3, A4, A5) const)
{
    return &SqMember<C, R>::template Func5C<A1, A2, A3, A4, A5, true>;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5>
inline SQFUNCTION SqMemberOverloadedFunc(R & (C::*method)(A1, A2, A3, A4, A5))
{
    return &SqMember<C, R&>::template Func5<A1, A2, A3, A4, A5, true>;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5>
inline SQFUNCTION SqMemberOverloadedFunc(R & (C::*method)(A1, A2, A3, A4, A5) const)
{
    return &SqMember<C, R&>::template Func5C<A1, A2, A3, A4, A5, true>;
}

// Arg Count 6
template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6>
inline SQFUNCTION SqMemberOverloadedFunc(R (C::*method)(A1, A2, A3, A4, A5, A6))
{
    return &SqMember<C, R>::template Func6<A1, A2, A3, A4, A5, A6, true>;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6>
inline SQFUNCTION SqMemberOverloadedFunc(R (C::*method)(A1, A2, A3, A4, A5, A6) const)
{
    return &SqMember<C, R>::template Func6C<A1, A2, A3, A4, A5, A6, true>;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6>
inline SQFUNCTION SqMemberOverloadedFunc(R & (C::*method)(A1, A2, A3, A4, A5, A6))
{
    return &SqMember<C, R&>::template Func6<A1, A2, A3, A4, A5, A6, true>;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6>
inline SQFUNCTION SqMemberOverloadedFunc(R & (C::*method)(A1, A2, A3, A4, A5, A6) const)
{
    return &SqMember<C, R&>::template Func6C<A1, A2, A3, A4, A5, A6, true>;
}

// Arg Count 7
template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
inline SQFUNCTION SqMemberOverloadedFunc(R (C::*method)(A1, A2, A3, A4, A5, A6, A7))
{
    return &SqMember<C, R>::template Func7<A1, A2, A3, A4, A5, A6, A7, true>;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
inline SQFUNCTION SqMemberOverloadedFunc(R (C::*method)(A1, A2, A3, A4, A5, A6, A7) const)
{
    return &SqMember<C, R>::template Func7C<A1, A2, A3, A4, A5, A6, A7, true>;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
inline SQFUNCTION SqMemberOverloadedFunc(R & (C::*method)(A1, A2, A3, A4, A5, A6, A7))
{
    return &SqMember<C, R&>::template Func7<A1, A2, A3, A4, A5, A6, A7, true>;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
inline SQFUNCTION SqMemberOverloadedFunc(R & (C::*method)(A1, A2, A3, A4, A5, A6, A7) const)
{
    return &SqMember<C, R&>::template Func7C<A1, A2, A3, A4, A5, A6, A7, true>;
}

// Arg Count 8
template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
inline SQFUNCTION SqMemberOverloadedFunc(R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8))
{
    return &SqMember<C, R>::template Func8<A1, A2, A3, A4, A5, A6, A7, A8, true>;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
inline SQFUNCTION SqMemberOverloadedFunc(R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8) const)
{
    return &SqMember<C, R>::template Func8C<A1, A2, A3, A4, A5, A6, A7, A8, true>;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
inline SQFUNCTION SqMemberOverloadedFunc(R & (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8))
{
    return &SqMember<C, R&>::template Func8<A1, A2, A3, A4, A5, A6, A7, A8, true>;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
inline SQFUNCTION SqMemberOverloadedFunc(R & (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8) const)
{
    return &SqMember<C, R&>::template Func8C<A1, A2, A3, A4, A5, A6, A7, A8, true>;
}

// Arg Count 9
template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
inline SQFUNCTION SqMemberOverloadedFunc(R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9))
{
    return &SqMember<C, R>::template Func9<A1, A2, A3, A4, A5, A6, A7, A8, A9, true>;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
inline SQFUNCTION SqMemberOverloadedFunc(R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9) const)
{
    return &SqMember<C, R>::template Func9C<A1, A2, A3, A4, A5, A6, A7, A8, A9, true>;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
inline SQFUNCTION SqMemberOverloadedFunc(R & (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9))
{
    return &SqMember<C, R&>::template Func9<A1, A2, A3, A4, A5, A6, A7, A8, A9, true>;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
inline SQFUNCTION SqMemberOverloadedFunc(R & (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9) const)
{
    return &SqMember<C, R&>::template Func9C<A1, A2, A3, A4, A5, A6, A7, A8, A9, true>;
}

// Arg Count 10
template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
inline SQFUNCTION SqMemberOverloadedFunc(R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10))
{
    return &SqMember<C, R>::template Func10<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, true>;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
inline SQFUNCTION SqMemberOverloadedFunc(R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10) const)
{
    return &SqMember<C, R>::template Func10C<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, true>;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
inline SQFUNCTION SqMemberOverloadedFunc(R & (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10))
{
    return &SqMember<C, R&>::template Func10<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, true>;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
inline SQFUNCTION SqMemberOverloadedFunc(R & (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10) const)
{
    return &SqMember<C, R&>::template Func10C<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, true>;
}

// Arg Count 11
template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
inline SQFUNCTION SqMemberOverloadedFunc(R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11))
{
    return &SqMember<C, R>::template Func11<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, true>;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
inline SQFUNCTION SqMemberOverloadedFunc(R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11) const)
{
    return &SqMember<C, R>::template Func11C<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, true>;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
inline SQFUNCTION SqMemberOverloadedFunc(R & (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11))
{
    return &SqMember<C, R&>::template Func11<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, true>;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
inline SQFUNCTION SqMemberOverloadedFunc(R & (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11) const)
{
    return &SqMember<C, R&>::template Func11C<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, true>;
}

// Arg Count 12
template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
inline SQFUNCTION SqMemberOverloadedFunc(R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12))
{
    return &SqMember<C, R>::template Func12<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, true>;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
inline SQFUNCTION SqMemberOverloadedFunc(R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12) const)
{
    return &SqMember<C, R>::template Func12C<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, true>;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
inline SQFUNCTION SqMemberOverloadedFunc(R & (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12))
{
    return &SqMember<C, R&>::template Func12<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, true>;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
inline SQFUNCTION SqMemberOverloadedFunc(R & (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12) const)
{
    return &SqMember<C, R&>::template Func12C<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, true>;
}

// Arg Count 13
template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13>
inline SQFUNCTION SqMemberOverloadedFunc(R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13))
{
    return &SqMember<C, R>::template Func13<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, true>;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13>
inline SQFUNCTION SqMemberOverloadedFunc(R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13) const)
{
    return &SqMember<C, R>::template Func13C<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, true>;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13>
inline SQFUNCTION SqMemberOverloadedFunc(R & (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13))
{
    return &SqMember<C, R&>::template Func13<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, true>;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13>
inline SQFUNCTION SqMemberOverloadedFunc(R & (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13) const)
{
    return &SqMember<C, R&>::template Func13C<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, true>;
}


// Arg Count 14
template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13, class A14>
inline SQFUNCTION SqMemberOverloadedFunc(R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14))
{
    return &SqMember<C, R>::template Func14<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, true>;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13, class A14>
inline SQFUNCTION SqMemberOverloadedFunc(R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14) const)
{
    return &SqMember<C, R>::template Func14C<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, true>;
}
template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13, class A14>
inline SQFUNCTION SqMemberOverloadedFunc(R & (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14))
{
    return &SqMember<C, R&>::template Func14<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, true>;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13, class A14>
inline SQFUNCTION SqMemberOverloadedFunc(R & (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14) const)
{
    return &SqMember<C, R&>::template Func14C<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, true>;
}

//
// Overload handler resolver
//

template <class R>
inline SQFUNCTION SqOverloadFunc(R (*method))
{
    return &SqOverload<R>::Func;
}

template <class C, class R>
inline SQFUNCTION SqOverloadFunc(R (C::*method))
{
    return &SqOverload<R>::Func;
}

template <class C, class R>
inline SQFUNCTION SqOverloadFunc(R (C::*method)() const )
{
    return &SqOverload<R>::Func;
}

template <class C, class R, class A1>
inline SQFUNCTION SqOverloadFunc(R (C::*method)(A1) const )
{
    return &SqOverload<R>::Func;
}

template <class C, class R, class A1, class A2>
inline SQFUNCTION SqOverloadFunc(R (C::*method)(A1, A2) const )
{
    return &SqOverload<R>::Func;
}

template <class C, class R, class A1, class A2, class A3>
inline SQFUNCTION SqOverloadFunc(R (C::*method)(A1, A2, A3) const )
{
    return &SqOverload<R>::Func;
}

template <class C, class R, class A1, class A2, class A3, class A4>
inline SQFUNCTION SqOverloadFunc(R (C::*method)(A1, A2, A3, A4) const )
{
    return &SqOverload<R>::Func;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5>
inline SQFUNCTION SqOverloadFunc(R (C::*method)(A1, A2, A3, A4, A5) const )
{
    return &SqOverload<R>::Func;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6>
inline SQFUNCTION SqOverloadFunc(R (C::*method)(A1, A2, A3, A4, A5, A6) const )
{
    return &SqOverload<R>::Func;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
inline SQFUNCTION SqOverloadFunc(R (C::*method)(A1, A2, A3, A4, A5, A6, A7) const )
{
    return &SqOverload<R>::Func;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
inline SQFUNCTION SqOverloadFunc(R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8) const )
{
    return &SqOverload<R>::Func;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
inline SQFUNCTION SqOverloadFunc(R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9) const )
{
    return &SqOverload<R>::Func;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
inline SQFUNCTION SqOverloadFunc(R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10) const )
{
    return &SqOverload<R>::Func;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
inline SQFUNCTION SqOverloadFunc(R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11) const )
{
    return &SqOverload<R>::Func;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
inline SQFUNCTION SqOverloadFunc(R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12) const )
{
    return &SqOverload<R>::Func;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13>
inline SQFUNCTION SqOverloadFunc(R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13) const )
{
    return &SqOverload<R>::Func;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13, class A14>
inline SQFUNCTION SqOverloadFunc(R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14) const )
{
    return &SqOverload<R>::Func;
}

//
// Query argument count
//

// Arg Count 0
template <class R>
inline int SqGetArgCount(R (*method)())
{
    return 0;
}

// Arg Count 1
template <class R, class A1>
inline int SqGetArgCount(R (*method)(A1))
{
    return 1;
}

// Arg Count 2
template <class R, class A1, class A2>
inline int SqGetArgCount(R (*method)(A1, A2))
{
    return 2;
}

// Arg Count 3
template <class R, class A1, class A2, class A3>
inline int SqGetArgCount(R (*method)(A1, A2, A3))
{
    return 3;
}

// Arg Count 4
template <class R, class A1, class A2, class A3, class A4>
inline int SqGetArgCount(R (*method)(A1, A2, A3, A4))
{
    return 4;
}

// Arg Count 5
template <class R, class A1, class A2, class A3, class A4, class A5>
inline int SqGetArgCount(R (*method)(A1, A2, A3, A4, A5))
{
    return 5;
}

// Arg Count 6
template <class R, class A1, class A2, class A3, class A4, class A5, class A6>
inline int SqGetArgCount(R (*method)(A1, A2, A3, A4, A5, A6))
{
    return 6;
}

// Arg Count 7
template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
inline int SqGetArgCount(R (*method)(A1, A2, A3, A4, A5, A6, A7))
{
    return 7;
}

// Arg Count 8
template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
inline int SqGetArgCount(R (*method)(A1, A2, A3, A4, A5, A6, A7, A8))
{
    return 8;
}

// Arg Count 9
template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
inline int SqGetArgCount(R (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9))
{
    return 9;
}

// Arg Count 10
template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
inline int SqGetArgCount(R (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10))
{
    return 10;
}

// Arg Count 11
template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
inline int SqGetArgCount(R (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11))
{
    return 11;
}

// Arg Count 12
template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
inline int SqGetArgCount(R (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12))
{
    return 12;
}

// Arg Count 13
template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13>
inline int SqGetArgCount(R (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13))
{
    return 13;
}

// Arg Count 14
template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13, class A14>
inline int SqGetArgCount(R (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14))
{
    return 14;
}

//
// Query member function argument count
//

// Arg Count 0
template <class C, class R>
inline int SqGetArgCount(R (C::*method)())
{
    return 0;
}

// Arg Count 1
template <class C, class R, class A1>
inline int SqGetArgCount(R (C::*method)(A1))
{
    return 1;
}

// Arg Count 2
template <class C, class R, class A1, class A2>
inline int SqGetArgCount(R (C::*method)(A1, A2))
{
    return 2;
}

// Arg Count 3
template <class C, class R, class A1, class A2, class A3>
inline int SqGetArgCount(R (C::*method)(A1, A2, A3))
{
    return 3;
}

// Arg Count 4
template <class C, class R, class A1, class A2, class A3, class A4>
inline int SqGetArgCount(R (C::*method)(A1, A2, A3, A4))
{
    return 4;
}

// Arg Count 5
template <class C, class R, class A1, class A2, class A3, class A4, class A5>
inline int SqGetArgCount(R (C::*method)(A1, A2, A3, A4, A5))
{
    return 5;
}

// Arg Count 6
template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6>
inline int SqGetArgCount(R (C::*method)(A1, A2, A3, A4, A5, A6))
{
    return 6;
}

// Arg Count 7
template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
inline int SqGetArgCount(R (C::*method)(A1, A2, A3, A4, A5, A6, A7))
{
    return 7;
}

// Arg Count 8
template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
inline int SqGetArgCount(R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8))
{
    return 8;
}

// Arg Count 9
template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
inline int SqGetArgCount(R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9))
{
    return 9;
}

// Arg Count 10
template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
inline int SqGetArgCount(R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10))
{
    return 10;
}

// Arg Count 11
template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
inline int SqGetArgCount(R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11))
{
    return 11;
}

// Arg Count 12
template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
inline int SqGetArgCount(R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12))
{
    return 12;
}

// Arg Count 13
template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13>
inline int SqGetArgCount(R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13))
{
    return 13;
}

// Arg Count 14
template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13, class A14>
inline int SqGetArgCount(R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14))
{
    return 14;
}

//
// Query const member function argument count
//

// Arg Count 0
template <class C, class R>
inline int SqGetArgCount(R (C::*method)() const)
{
    return 0;
}

// Arg Count 1
template <class C, class R, class A1>
inline int SqGetArgCount(R (C::*method)(A1) const)
{
    return 1;
}

// Arg Count 2
template <class C, class R, class A1, class A2>
inline int SqGetArgCount(R (C::*method)(A1, A2) const)
{
    return 2;
}

// Arg Count 3
template <class C, class R, class A1, class A2, class A3>
inline int SqGetArgCount(R (C::*method)(A1, A2, A3) const)
{
    return 3;
}

// Arg Count 4
template <class C, class R, class A1, class A2, class A3, class A4>
inline int SqGetArgCount(R (C::*method)(A1, A2, A3, A4) const)
{
    return 4;
}

// Arg Count 5
template <class C, class R, class A1, class A2, class A3, class A4, class A5>
inline int SqGetArgCount(R (C::*method)(A1, A2, A3, A4, A5) const)
{
    return 5;
}

// Arg Count 6
template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6>
inline int SqGetArgCount(R (C::*method)(A1, A2, A3, A4, A5, A6) const)
{
    return 6;
}

// Arg Count 7
template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
inline int SqGetArgCount(R (C::*method)(A1, A2, A3, A4, A5, A6, A7) const)
{
    return 7;
}

// Arg Count 8
template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
inline int SqGetArgCount(R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8) const)
{
    return 8;
}

// Arg Count 9
template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
inline int SqGetArgCount(R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9) const)
{
    return 9;
}

// Arg Count 10
template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
inline int SqGetArgCount(R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10) const)
{
    return 10;
}

// Arg Count 11
template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
inline int SqGetArgCount(R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11) const)
{
    return 11;
}

// Arg Count 12
template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
inline int SqGetArgCount(R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12) const)
{
    return 12;
}

// Arg Count 13
template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13>
inline int SqGetArgCount(R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13) const)
{
    return 13;
}

// Arg Count 14
template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13, class A14>
inline int SqGetArgCount(R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14) const)
{
    return 14;
}

//
// Helper function used for binding overload handlers
//

// TODO

//
// Bind overload handler for normal functions
//

// Arg Count 0
template <class R>
inline void SqBindOverloadHandler(HSQUIRRELVM vm, R (*method)(), SQFUNCTION func)
{
    sq_pushinteger(vm, 0);
    SQUserPointer methodPtr = sq_newuserdata(vm, static_cast<SQUnsignedInteger>(sizeof(method)));
    memcpy(methodPtr, &method, sizeof(method));
    sq_newclosure(vm, func, 1);
    sq_newslot(vm, -3, false);
}

// Arg Count 1
template <class R, class A1>
inline void SqBindOverloadHandler(HSQUIRRELVM vm, R (*method)(A1), SQFUNCTION func)
{
    // TODO
}

// Arg Count 2
template <class R, class A1, class A2>
inline void SqBindOverloadHandler(HSQUIRRELVM vm, R (*method)(A1, A2), SQFUNCTION func)
{
    // TODO
}

// Arg Count 3
template <class R, class A1, class A2, class A3>
inline void SqBindOverloadHandler(HSQUIRRELVM vm, R (*method)(A1, A2, A3), SQFUNCTION func)
{
    // TODO
}

// Arg Count 4
template <class R, class A1, class A2, class A3, class A4>
inline void SqBindOverloadHandler(HSQUIRRELVM vm, R (*method)(A1, A2, A3, A4), SQFUNCTION func)
{
    // TODO
}

// Arg Count 5
template <class R, class A1, class A2, class A3, class A4, class A5>
inline void SqBindOverloadHandler(HSQUIRRELVM vm, R (*method)(A1, A2, A3, A4, A5), SQFUNCTION func)
{
    // TODO
}

// Arg Count 6
template <class R, class A1, class A2, class A3, class A4, class A5, class A6>
inline void SqBindOverloadHandler(HSQUIRRELVM vm, R (*method)(A1, A2, A3, A4, A5, A6), SQFUNCTION func)
{
    // TODO
}

// Arg Count 7
template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
inline void SqBindOverloadHandler(HSQUIRRELVM vm, R (*method)(A1, A2, A3, A4, A5, A6, A7), SQFUNCTION func)
{
    // TODO
}

// Arg Count 8
template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
inline void SqBindOverloadHandler(HSQUIRRELVM vm, R (*method)(A1, A2, A3, A4, A5, A6, A7, A8), SQFUNCTION func)
{
    // TODO
}

// Arg Count 9
template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
inline void SqBindOverloadHandler(HSQUIRRELVM vm, R (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9), SQFUNCTION func)
{
    // TODO
}

// Arg Count 10
template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
inline void SqBindOverloadHandler(HSQUIRRELVM vm, R (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10), SQFUNCTION func)
{
    // TODO
}

// Arg Count 11
template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
inline void SqBindOverloadHandler(HSQUIRRELVM vm, R (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11), SQFUNCTION func)
{
    // TODO
}

// Arg Count 12
template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
inline void SqBindOverloadHandler(HSQUIRRELVM vm, R (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12), SQFUNCTION func)
{
    // TODO
}

// Arg Count 13
template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13>
inline void SqBindOverloadHandler(HSQUIRRELVM vm, R (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13), SQFUNCTION func)
{
    // TODO
}

// Arg Count 14
template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13, class A14>
inline void SqBindOverloadHandler(HSQUIRRELVM vm, R (*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14), SQFUNCTION func)
{
    // TODO
}

//
// Bind overload handler for member functions
//

// Arg Count 0
template <class C, class R>
inline void SqBindOverloadHandler(HSQUIRRELVM vm, R (C::*method)(), SQFUNCTION func)
{
    sq_pushinteger(vm, 0);
    SQUserPointer methodPtr = sq_newuserdata(vm, static_cast<SQUnsignedInteger>(sizeof(method)));
    memcpy(methodPtr, &method, sizeof(method));
    sq_newclosure(vm, func, 1);
    sq_newslot(vm, -3, false);
}

// Arg Count 1
template <class C, class R, class A1>
inline void SqBindOverloadHandler(HSQUIRRELVM vm, R (C::*method)(A1), SQFUNCTION func)
{
    // TODO
}

// Arg Count 2
template <class C, class R, class A1, class A2>
inline void SqBindOverloadHandler(HSQUIRRELVM vm, R (C::*method)(A1, A2), SQFUNCTION func)
{
    // TODO
}

// Arg Count 3
template <class C, class R, class A1, class A2, class A3>
inline void SqBindOverloadHandler(HSQUIRRELVM vm, R (C::*method)(A1, A2, A3), SQFUNCTION func)
{
    // TODO
}

// Arg Count 4
template <class C, class R, class A1, class A2, class A3, class A4>
inline void SqBindOverloadHandler(HSQUIRRELVM vm, R (C::*method)(A1, A2, A3, A4), SQFUNCTION func)
{
    // TODO
}

// Arg Count 5
template <class C, class R, class A1, class A2, class A3, class A4, class A5>
inline void SqBindOverloadHandler(HSQUIRRELVM vm, R (C::*method)(A1, A2, A3, A4, A5), SQFUNCTION func)
{
    // TODO
}

// Arg Count 6
template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6>
inline void SqBindOverloadHandler(HSQUIRRELVM vm, R (C::*method)(A1, A2, A3, A4, A5, A6), SQFUNCTION func)
{
    // TODO
}

// Arg Count 7
template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
inline void SqBindOverloadHandler(HSQUIRRELVM vm, R (C::*method)(A1, A2, A3, A4, A5, A6, A7), SQFUNCTION func)
{
    // TODO
}

// Arg Count 8
template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
inline void SqBindOverloadHandler(HSQUIRRELVM vm, R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8), SQFUNCTION func)
{
    // TODO
}

// Arg Count 9
template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
inline void SqBindOverloadHandler(HSQUIRRELVM vm, R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9), SQFUNCTION func)
{
    // TODO
}

// Arg Count 10
template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
inline void SqBindOverloadHandler(HSQUIRRELVM vm, R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10), SQFUNCTION func)
{
    // TODO
}

// Arg Count 11
template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
inline void SqBindOverloadHandler(HSQUIRRELVM vm, R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11), SQFUNCTION func)
{
    // TODO
}

// Arg Count 12
template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
inline void SqBindOverloadHandler(HSQUIRRELVM vm, R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12), SQFUNCTION func)
{
    // TODO
}

// Arg Count 13
template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13>
inline void SqBindOverloadHandler(HSQUIRRELVM vm, R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13), SQFUNCTION func)
{
    // TODO
}

// Arg Count 14
template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13, class A14>
inline void SqBindOverloadHandler(HSQUIRRELVM vm, R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14), SQFUNCTION func)
{
    // TODO
}

//
// Bind overload handler for const member functions
//

// Arg Count 0
template <class C, class R>
inline void SqBindOverloadHandler(HSQUIRRELVM vm, R (C::*method)() const, SQFUNCTION func)
{
    sq_pushinteger(vm, 0);
    SQUserPointer methodPtr = sq_newuserdata(vm, static_cast<SQUnsignedInteger>(sizeof(method)));
    memcpy(methodPtr, &method, sizeof(method));
    sq_newclosure(vm, func, 1);
    sq_newslot(vm, -3, false);
}

// Arg Count 1
template <class C, class R, class A1>
inline void SqBindOverloadHandler(HSQUIRRELVM vm, R (C::*method)(A1) const, SQFUNCTION func)
{
    // TODO
}

// Arg Count 2
template <class C, class R, class A1, class A2>
inline void SqBindOverloadHandler(HSQUIRRELVM vm, R (C::*method)(A1, A2) const, SQFUNCTION func)
{
    // TODO
}

// Arg Count 3
template <class C, class R, class A1, class A2, class A3>
inline void SqBindOverloadHandler(HSQUIRRELVM vm, R (C::*method)(A1, A2, A3) const, SQFUNCTION func)
{
    // TODO
}

// Arg Count 4
template <class C, class R, class A1, class A2, class A3, class A4>
inline void SqBindOverloadHandler(HSQUIRRELVM vm, R (C::*method)(A1, A2, A3, A4) const, SQFUNCTION func)
{
    // TODO
}

// Arg Count 5
template <class C, class R, class A1, class A2, class A3, class A4, class A5>
inline void SqBindOverloadHandler(HSQUIRRELVM vm, R (C::*method)(A1, A2, A3, A4, A5) const, SQFUNCTION func)
{
    // TODO
}

// Arg Count 6
template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6>
inline void SqBindOverloadHandler(HSQUIRRELVM vm, R (C::*method)(A1, A2, A3, A4, A5, A6) const, SQFUNCTION func)
{
    // TODO
}

// Arg Count 7
template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
inline void SqBindOverloadHandler(HSQUIRRELVM vm, R (C::*method)(A1, A2, A3, A4, A5, A6, A7) const, SQFUNCTION func)
{
    // TODO
}

// Arg Count 8
template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
inline void SqBindOverloadHandler(HSQUIRRELVM vm, R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8) const, SQFUNCTION func)
{
    // TODO
}

// Arg Count 9
template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
inline void SqBindOverloadHandler(HSQUIRRELVM vm, R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9) const, SQFUNCTION func)
{
    // TODO
}

// Arg Count 10
template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
inline void SqBindOverloadHandler(HSQUIRRELVM vm, R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10) const, SQFUNCTION func)
{
    // TODO
}

// Arg Count 11
template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
inline void SqBindOverloadHandler(HSQUIRRELVM vm, R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11) const, SQFUNCTION func)
{
    // TODO
}

// Arg Count 12
template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
inline void SqBindOverloadHandler(HSQUIRRELVM vm, R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12) const, SQFUNCTION func)
{
    // TODO
}

// Arg Count 13
template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13>
inline void SqBindOverloadHandler(HSQUIRRELVM vm, R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13) const, SQFUNCTION func)
{
    // TODO
}

// Arg Count 14
template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13, class A14>
inline void SqBindOverloadHandler(HSQUIRRELVM vm, R (C::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14) const, SQFUNCTION func)
{
    // TODO
}

}

#endif
