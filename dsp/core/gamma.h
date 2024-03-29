#ifndef COSMOPOLITAN_DSP_CORE_GAMMA_H_
#define COSMOPOLITAN_DSP_CORE_GAMMA_H_
#include "libc/math.h"

#define COMPANDLUMA(X, ...) COMPANDLUMA_(X, __VA_ARGS__)
#define COMPANDLUMA_(X, K1, K2, K3, K4) \
  ((X) > (K3) / (K4) ? (1 + (K2)) * pow((X), 1 / (K1)) - (K2) : (X) * (K4))

#define UNCOMPANDLUMA(X, ...) UNCOMPANDLUMA_(X, __VA_ARGS__)
#define UNCOMPANDLUMA_(X, K1, K2, K3, K4) \
  ((X) > (K3) ? pow(1 / (1 + (K2)) * ((X) + (K2)), K1) : (X) / (K4))

#define COMPANDLUMA_SRGB_MAGNUM  .055, .04045, 12.92
#define COMPANDLUMA_SRGB(X, G)   COMPANDLUMA(X, G, COMPANDLUMA_SRGB_MAGNUM)
#define UNCOMPANDLUMA_SRGB(X, G) UNCOMPANDLUMA(X, G, COMPANDLUMA_SRGB_MAGNUM)

#define COMPANDLUMA_BT1886_MAGNUM 1 / .45, .099, .081, 4.5
#define COMPANDLUMA_BT1886(X)     COMPANDLUMA(X, COMPANDLUMA_BT1886_MAGNUM)
#define UNCOMPANDLUMA_BT1886(X)   UNCOMPANDLUMA(X, COMPANDLUMA_BT1886_MAGNUM)

#endif /* COSMOPOLITAN_DSP_CORE_GAMMA_H_ */
