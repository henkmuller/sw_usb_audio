#ifdef DSP_MULTI_THREAD

#include "dsp.h"
#include "xcore/chanend.h"
#include "xcore/channel.h"
#define NUM_INPUTS 1
#define NUM_OUTPUTS 2

static chanend_t g_c, g_c2;
  
void UserBufferManagement(
    unsigned output_samples[NUM_OUTPUTS],
    unsigned  input_samples[NUM_INPUTS]
    ) {
    chan_out_buf_word(g_c, output_samples, NUM_OUTPUTS);
    chan_out_buf_word(g_c, input_samples,  NUM_INPUTS);
    chan_in_buf_word( g_c, output_samples, NUM_OUTPUTS/2);
    chan_in_buf_word( g_c, input_samples,  NUM_INPUTS/2);
    chan_out_buf_word(g_c, output_samples, NUM_OUTPUTS);
    chan_out_buf_word(g_c, input_samples,  NUM_INPUTS);
    chan_in_buf_word( g_c, output_samples+NUM_OUTPUTS/2, NUM_OUTPUTS/2);
    chan_in_buf_word( g_c, input_samples+NUM_INPUTS/2,  NUM_INPUTS/2);
}

void UserBufferManagementSetChan(chanend_t c, chanend_t c2) {
    g_c = c;
    g_c2 = c2;
}


void UserBufferManagementInit() {}

static int32_t filter_coeffs[4*5] = {
    0x10000000,  0x10000000,  0x10000000,  0x10000000,  0x10000000,
    0x10000000,  0x10000000,  0x10000000,  0x10000000,  0x10000000,
    0x10000000,  0x10000000,  0x10000000,  0x10000000,  0x10000000,
    0x10000000,  0x10000000,  0x10000000,  0x10000000,  0x10000000,
};

static int32_t filter_states[2][4*4];

static int32_t filter_states2[2][4*4];

void dsp_main1(chanend_t c_data) {
    int for_usb [NUM_INPUTS/2 + NUM_OUTPUTS/2];
    int from_usb[NUM_INPUTS + NUM_OUTPUTS];
    while(1) {
        chan_in_buf_word( c_data, &for_usb[0],           NUM_OUTPUTS);
        chan_in_buf_word( c_data, &for_usb[NUM_OUTPUTS], NUM_INPUTS);
        chan_out_buf_word(c_data, &from_usb[0],          NUM_OUTPUTS/2);
        chan_out_buf_word(c_data, &from_usb[NUM_OUTPUTS/2],NUM_INPUTS/2);
        for(int i = 0; i < 2; i++) {
            for_usb[i] = dsp_filters_biquads((int32_t) from_usb[i],
                                             filter_coeffs,
                                             filter_states[i],
                                             4,
                                             28);
        }
    }
}

void dsp_main2(chanend_t c_data) {
    int for_usb [NUM_INPUTS/2 + NUM_OUTPUTS/2];
    int from_usb[NUM_INPUTS + NUM_OUTPUTS];
    while(1) {
        chan_in_buf_word( c_data, &for_usb[0],           NUM_OUTPUTS);
        chan_in_buf_word( c_data, &for_usb[NUM_OUTPUTS], NUM_INPUTS);
        chan_out_buf_word(c_data, &from_usb[0],          NUM_OUTPUTS/2);
        chan_out_buf_word(c_data, &from_usb[NUM_OUTPUTS/2],NUM_INPUTS/2);
        for(int i = 0; i < 2; i++) {
            for_usb[i] = dsp_filters_biquads((int32_t) from_usb[i],
                                             filter_coeffs,
                                             filter_states2[i],
                                             4,
                                             28);
        }
    }
}

#endif // DSP_MULTI_THREAD
