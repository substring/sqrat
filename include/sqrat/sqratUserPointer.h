//
// sqratUserPointer: SQUserPointer support
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

#if !defined(_SCRAT_USERPOINTER_H_)
#define _SCRAT_USERPOINTER_H_

#include "sqratTypes.h"

namespace Sqrat {
	template<>
	struct Var<SQUserPointer> {

		SQUserPointer value; ///< The actual value of get operations

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/// Attempts to get the value off the stack at idx as a SQUserPointer
		///
		/// \param vm  Target VM
		/// \param idx Index trying to be read
		///
		/// \remarks
		/// This function MUST have its Error handled if it occurred.
		///
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		Var(HSQUIRRELVM vm, SQInteger idx) {
			SQUserPointer sqValue;
#if !defined (SCRAT_NO_ERROR_CHECKING)
			if (SQ_FAILED(sq_getuserpointer(vm, idx, &sqValue))) {
				SQTHROW(vm, FormatTypeError(vm, idx, _SC("userpointer")));
			}
#else
			sq_getuserpointer(vm, idx, &sqValue);
#endif
			value = sqValue;
		}

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/// Called by Sqrat::PushVar to put a SQUserPointer on the stack
		///
		/// \param vm    Target VM
		/// \param value Value to push on to the VM's stack
		///
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		static void push(HSQUIRRELVM vm, const SQUserPointer value) {
			sq_pushuserpointer(vm, value);
		}
	};
}
#endif