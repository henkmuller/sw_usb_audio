#ifdef DSP_SINGLE_THREAD

#include "dsp.h"
#include "xcore/chanend.h"
#include "xcore/channel.h"
#include <print.h>

#define NUM_INPUTS  4
#define NUM_OUTPUTS 2

//:ustart
static chanend_t g_c;
  
void UserBufferManagement(
    unsigned output_samples[NUM_OUTPUTS],
    unsigned  input_samples[NUM_INPUTS]
    ) {
    chan_out_buf_word(g_c, output_samples, NUM_OUTPUTS);
    chan_out_buf_word(g_c, input_samples,  NUM_INPUTS);
    chan_in_buf_word( g_c, output_samples, NUM_OUTPUTS);
    chan_in_buf_word( g_c, input_samples,  NUM_INPUTS);
}

void UserBufferManagementSetChan(chanend_t c) {
    g_c = c;
}

void UserBufferManagementInit() {}
//:uend

//:dstart
#define FILTERS 4
//    b2/a0      b1/a0       b0/a0      -a1/a0     -a2/a0
int32_t filter_coeffs[FILTERS*5] = {
    261565110, -521424736, 260038367, 521424736, -253168021,
    255074543, -506484921, 252105451, 506484921, -238744538,
    280274501, -523039333, 245645878, 523039333, -257484924,
    291645146, -504140302, 223757950, 504140302, -246967640,
};

int32_t filter_states[NUM_INPUTS+NUM_OUTPUTS][FILTERS*4];

void dsp_main(chanend_t c_data) {
    int for_usb [NUM_INPUTS + NUM_OUTPUTS];
    int from_usb[NUM_INPUTS + NUM_OUTPUTS];
    while(1) {
        chan_in_buf_word( c_data, &from_usb[0],          NUM_OUTPUTS);
        chan_in_buf_word( c_data, &from_usb[NUM_OUTPUTS],NUM_INPUTS);
        chan_out_buf_word(c_data, &for_usb[0],           NUM_OUTPUTS);
        chan_out_buf_word(c_data, &for_usb[NUM_OUTPUTS], NUM_INPUTS);
        for(int i = 0; i < 2; i++) {
            for_usb[i] = dsp_filters_biquads((int32_t) from_usb[i],
                                             filter_coeffs,
                                             filter_states[i],
                                             FILTERS,
                                             28);
        }
    }
}
//:dend

#endif // DSP_SINGLE_THREAD
