#ifdef DSP_USB_THREAD

#include "dsp.h"

#define NUM_INPUTS  4
#define NUM_OUTPUTS 2

extern void UserBufferManagement(
    unsigned output_samples[NUM_OUTPUTS],
    unsigned  input_samples[NUM_INPUTS]);
//:end

extern void UserBufferManagementInit(void);
//:end

//:start
#define FILTERS 4
//    b2/a0      b1/a0       b0/a0      -a1/a0     -a2/a0
int32_t filter_coeffs[FILTERS*5] = {
    261565110, -521424736, 260038367, 521424736, -253168021,
    255074543, -506484921, 252105451, 506484921, -238744538,
    280274501, -523039333, 245645878, 523039333, -257484924,
    291645146, -504140302, 223757950, 504140302, -246967640,
};

int32_t filter_states[NUM_INPUTS+NUM_OUTPUTS][FILTERS*4];

void UserBufferManagement(
    unsigned output_samples[NUM_OUTPUTS],
    unsigned  input_samples[NUM_INPUTS])
{
    for(int i = 0; i < NUM_OUTPUTS; i++) {
        output_samples[i] = dsp_filters_biquads((int32_t) output_samples[i],
                                               filter_coeffs,
                                               filter_states[i],
                                               FILTERS,
                                               28); 
    }
}

void UserBufferManagementInit() {}
//:end

#endif // DSP_USB_THREAD
