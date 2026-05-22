#include <cgfx.h>

asm error_code
cgfx_ss_mssig(path_id path, int signo)
{
    asm
    {
		ldx		2+2,s		get signal number
		lda		2+1,s		get path
		ldb		#SS_MsSig
		os9		I$SetStt
    }
}
