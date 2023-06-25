#ifdef DSP_USB_THREAD

#include "dsp.h"

#define NUM_INPUTS 1
#define NUM_OUTPUTS 2

extern void UserBufferManagement(
    unsigned output_samples[NUM_OUTPUTS],
    unsigned  input_samples[NUM_INPUTS]);
//:end

extern void UserBufferManagementInit(void);
//:end

//:start
int32_t filter_coeffs[4*5] = {
    0x10000000,  0x10000000,  0x10000000,  0x10000000,  0x10000000,
    0x10000000,  0x10000000,  0x10000000,  0x10000000,  0x10000000,
    0x10000000,  0x10000000,  0x10000000,  0x10000000,  0x10000000,
    0x10000000,  0x10000000,  0x10000000,  0x10000000,  0x10000000,
};

int32_t filter_states[2][4*4];

void UserBufferManagement(
    unsigned output_samples[NUM_OUTPUTS],
    unsigned  input_samples[NUM_INPUTS])
{
    for(int i = 0; i < 2; i++) {
        output_samples[i] = dsp_filters_biquads((int32_t) output_samples[i],
                                               filter_coeffs,
                                               filter_states[i],
                                               4,
                                               28);
    }
}

void UserBufferManagementInit() {}
//:end

#endif // DSP_USB_THREAD
