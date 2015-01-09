
/* test if compiler support '%L' modifier for float pointer number
 * gcc needs '-posix' or '-D__USE_MINGW_ANSI_STDIO' flags to support
 * posix 'printf()', the win32 crt cannot work properly here
 */
#include <stdio.h>
#include <float.h>
int main (void)
{
	long double d = 1. / 31.;
	printf ("%Lf\n", d);
	printf ("%e\n", FLT_EPSILON);
	printf ("%e\n", DBL_EPSILON);
	return 0;
}

/* expected output: 0.032258 */
