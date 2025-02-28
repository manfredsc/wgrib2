/**
 * Header file with internal function prototypes for the wgrib2 extra subdirectory.
 * 
 * Alyson Stahl 2/28/2025
 */

struct extraftn {const char *name; int (*fn)(); enum fntype type; int nargs; const char *desc; int sort;};

extern struct extraftn extraftns[];

extern int nextraftns;

int f_mysql(int mode, unsigned char **sec, float *data, unsigned int ndata, char *inv, void **local,
            const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5);
int f_mysql_dump(int mode, unsigned char **sec, float *data, unsigned int ndata, char *inv, void **local,
                const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5,
                const char *arg6, const char *arg7);
int f_mysql_speed(int mode, unsigned char **sec, float *data, unsigned int ndata, char *inv, void **local, 
                const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5,
                const char *arg6, const char *arg7);