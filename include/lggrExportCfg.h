/*********************************************************************************************
 *	Name		: LGGRExportCfg.h
 *	Description	: Configuration of the project
 *	Copyright	(C) 2024  Ignacio Pomar Ballestero
 ********************************************************************************************/

#pragma once
#ifndef _LGGR_CFG_H_
#	define _LGGR_CFG_H_

// If the solution is a dinamic library (dll), we need the next macro
#	define LGGR_DLL

// IMPORTANT: the project who exports must have the preprocessor macro STREAMLOGGER_EXPORTS

// see http://gcc.gnu.org/wiki/Visibility

// Generic helper definitions for shared library support
#	if defined _WIN32 || defined __CYGWIN__
#		define LGGR_HELPER_DLL_IMPORT __declspec (dllimport)
#		define LGGR_HELPER_DLL_EXPORT __declspec (dllexport)
#		define LGGR_HELPER_DLL_LOCAL
#	else
#		if __GNUC__ >= 4
#			define LGGR_HELPER_DLL_IMPORT __attribute__ ((visibility ("default")))
#			define LGGR_HELPER_DLL_EXPORT __attribute__ ((visibility ("default")))
#			define LGGR_HELPER_DLL_LOCAL  __attribute__ ((visibility ("hidden")))
#		else
#			define LGGR_HELPER_DLL_IMPORT
#			define LGGR_HELPER_DLL_EXPORT
#			define LGGR_HELPER_DLL_LOCAL
#		endif
#	endif

// Now we use the generic helper definitions above to define LGGR_API and LGGR_LOCAL.
// LGGR_API is used for the public API symbols. It either DLL imports or DLL exports (or does nothing for static build)
// LGGR_LOCAL is used for non-api symbols.

#	ifdef LGGR_DLL                    // defined if LGGR is compiled as a DLL
#		ifdef STREAMLOGGER_EXPORTS    // defined if we are building the LGGR DLL (instead of using it)
#			define LGGR_API LGGR_HELPER_DLL_EXPORT
#		else
#			define LGGR_API LGGR_HELPER_DLL_IMPORT
#		endif    // STREAMLOGGER_EXPORTS
#		define LGGR_LOCAL LGGR_HELPER_DLL_LOCAL
#	else    // LGGR_DLL is not defined: this means LGGR is a static lib.
#		define LGGR_API
#		define LGGR_LOCAL
#	endif    // LGGR_DLL

#	ifdef __GNUC__
#		define DEPRECATED __attribute__ ((deprecated))
#	elif defined(_MSC_VER)
#		define DEPRECATED __declspec (deprecated)
#	else
#		pragma message("WARNING: You need to implement DEPRECATED for this compiler")
#		define DEPRECATED
#	endif

#endif    //_LGGR_CFG_H_
