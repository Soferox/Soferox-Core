

#define main soferoxd_main
#include "soferoxd.cpp"

extern "C" __declspec(dllexport) void SoferoxdEntry() {
	exit(soferoxd_main(__argc, __argv));
}

#undef main
#define main soferox_cli_main
#include "soferox-cli.cpp"

extern "C" __declspec(dllexport) void SoferoxCliEntry() {
	exit(soferox_cli_main(__argc, __argv));
}

#undef main
#define main soferox_tx_main
#include "soferox-tx.cpp"

extern "C" __declspec(dllexport) void SoferoxTxEntry() {
	exit(soferox_tx_main(__argc, __argv));
}

#undef main
#include "qt/bitcoin.cpp"

extern "C" WORD __cdecl __scrt_get_show_window_mode();

extern "C" __declspec(dllexport) void SoferoxQtEntry() {
	ExitProcess(wWinMain(GetModuleHandle(0), nullptr, _get_wide_winmain_command_line(), __scrt_get_show_window_mode()));
}
