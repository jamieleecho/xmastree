#ifndef _STDBOOL_H
#define _STDBOOL_H

/* cmoc has no <stdbool.h>; MVKit ships this shim so the framework is
   self-contained. The include guard matches the app's own stdbool.h so the two
   never produce a duplicate `bool` typedef in one translation unit. */
typedef enum {
    false = 0,
    true = 1
} bool;

#endif /* _STDBOOL_H */
