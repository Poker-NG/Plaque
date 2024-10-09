#ifndef _APP_RNG_H
#define _APP_RNG_H

// This should not be used for secure purposes. (It won't because the plaque only holds informations this is used to chose fonts and so on.)

void rng_init(void);
unsigned int rng_read_raw(void);
void rng_do_seed_quick(void);
void rng_do_seed_long(void);

#endif // !_APP_RNG_H
