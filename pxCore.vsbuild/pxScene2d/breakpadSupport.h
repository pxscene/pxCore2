#pragma once

#ifdef HAS_BREAKPAD

#ifdef PX_PLATFORM_LINUX
#include "client/linux/handler/exception_handler.h"

static bool dumpCallback(const google_breakpad::MinidumpDescriptor& descriptor,
	void* context, bool succeeded) {
	return succeeded;
}

void enableBreakpad() 
{
	google_breakpad::MinidumpDescriptor descriptor("/tmp");
	google_breakpad::ExceptionHandler eh(descriptor, NULL, dumpCallback, NULL, true, -1);
}

#elif PX_PLATFORM_WIN
#include "client/windows/handler/exception_handler.h"

static bool dumpCallback(const wchar_t* dump_path,
	const wchar_t* minidump_id,
	void* context,
	EXCEPTION_POINTERS* exinfo,
	MDRawAssertionInfo* assertion,
	bool succeeded) {
	return succeeded;
}

void enableBreakpad()
{
	wchar_t buf[MAX_PATH];
	GetTempPathW(MAX_PATH, buf);
	const std::wstring tmpdir(buf);
	google_breakpad::ExceptionHandler eh(tmpdir, NULL, dumpCallback, NULL, google_breakpad::ExceptionHandler::HandlerType::HANDLER_ALL);
}
#else
void enableBreakpad() 
{
	//NOOP 
}
#endif
#endif