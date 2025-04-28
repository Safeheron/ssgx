#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

int __month_to_secs(int, int);
long long __year_to_secs(long long, int*);
long long __tm_to_secs(const struct tm*);
int __secs_to_tm(long long, struct tm*);

#ifdef __cplusplus
}
#endif /* __cplusplus */
