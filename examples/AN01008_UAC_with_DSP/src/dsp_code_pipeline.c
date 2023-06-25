#ifdef DSP_PIPELINE

#include "dsp.h"
#include "xcore/chanend.h"
#include "xcore/channel.h"
#include "xcore/parallel.h"
#define NUM_INPUTS 1
#define NUM_OUTPUTS 2

//:dmainstart  
DECLARE_JOB(dsp_data_distributor, (chanend_t, chanend_t, chanend_t));
DECLARE_JOB(dsp_thread0,  (chanend_t, chanend_t, chanend_t));
DECLARE_JOB(dsp_thread1a, (chanend_t, chanend_t));
DECLARE_JOB(dsp_thread1b, (chanend_t, chanend_t));
DECLARE_JOB(dsp_thread2,  (chanend_t, chanend_t, chanend_t));

void dsp_main(chanend_t c_data) {
    channel_t c_dist_to_0 = chan_alloc();
    channel_t c_0_to_1a   = chan_alloc();
    channel_t c_0_to_1b   = chan_alloc();
    channel_t c_1a_to_2   = chan_alloc();
    channel_t c_1b_to_2   = chan_alloc();
    channel_t c_2_to_dist = chan_alloc();
    PAR_JOBS(
        PJOB(dsp_data_distributor, (c_data, c_dist_to_0.end_a, c_2_to_dist.end_b)),
        PJOB(dsp_thread0,  (c_dist_to_0.end_b, c_0_to_1a.end_a, c_0_to_1b.end_a)),
        PJOB(dsp_thread1a, (c_0_to_1a.end_b, c_1a_to_2.end_a)),
        PJOB(dsp_thread1b, (c_0_to_1b.end_b, c_1b_to_2.end_a)),
        PJOB(dsp_thread2,  (c_1a_to_2.end_b, c_1a_to_2.end_b, c_2_to_dist.end_a))
        );
}
//:dmainend

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

void UserBufferManagementSetChan(chanend_t c) {
    g_c = c;
}


void UserBufferManagementInit() {}


//:dsp0start
static int32_t filter_coeffs0[1*5] = {
    0x10000000,  0x10000000,  0x10000000,  0x10000000,  0x10000000,
};

static int32_t filter_states0[2][1*4];

void dsp_thread0(chanend_t c_fromusb,
                 chanend_t c_to1a, chanend_t c_to1b) {     
    int from_usb[NUM_OUTPUTS];
    int for_1[NUM_OUTPUTS];
    while(1) {
        // Pick up my chunk of data to work on
        chan_in_buf_word(c_fromusb, &from_usb[0], NUM_OUTPUTS);

        for(int i = 0; i < 2; i++) {
            for_1[i] = dsp_filters_biquads((int32_t) from_usb[i],
                                          filter_coeffs0,
                                          filter_states0[i],
                                          1,
                                          28);
        }
      
        // And deliver my answer back
        chan_out_buf_word(c_to1a, &for_1[0], NUM_OUTPUTS);
        chan_out_buf_word(c_to1b, &for_1[0], NUM_OUTPUTS);
    }
}
//:dsp0end

//:dsp1astart
static int32_t filter_states1a[2][2*4];

static int32_t filter_coeffs1a[2*5] = {
    0x10000000,  0x10000000,  0x10000000,  0x10000000,  0x10000000,
    0x10000000,  0x10000000,  0x10000000,  0x10000000,  0x10000000,
};

void dsp_thread1a(chanend_t c_from0,
                  chanend_t c_to2) {     
    int from_0[NUM_OUTPUTS];
    int for_2[NUM_OUTPUTS];
    while(1) {
        // Pick up my chunk of data to work on
        chan_in_buf_word(c_from0, &from_0[0], NUM_OUTPUTS);

        for(int i = 0; i < 2; i++) {
            for_2[i] = dsp_filters_biquads((int32_t) from_0[i],
                                          filter_coeffs1a,
                                          filter_states1a[i],
                                          2,
                                          28);
        }
      
        // And deliver my answer back
        chan_out_buf_word(c_to2, &for_2[0], NUM_OUTPUTS);
    }
}
//:dsp1aend

//:dsp1bstart
static int32_t filter_states1b[2][2*4];

static int32_t filter_coeffs1b[2*5] = {
    0x10000000,  0x10000000,  0x10000000,  0x10000000,  0x10000000,
    0x10000000,  0x10000000,  0x10000000,  0x10000000,  0x10000000,
};

void dsp_thread1b(chanend_t c_from0,
                  chanend_t c_to2) {     
    int from_0[NUM_OUTPUTS];
    int for_2[NUM_OUTPUTS];
    while(1) {
        // Pick up my chunk of data to work on
        chan_in_buf_word(c_from0, &from_0[0], NUM_OUTPUTS);


        for(int i = 0; i < 2; i++) {
            for_2[i] = dsp_filters_biquads((int32_t) from_0[i],
                                          filter_coeffs1b,
                                          filter_states1b[i],
                                          2,
                                          28);
        }
      
        // And deliver my answer back
        chan_out_buf_word(c_to2, &for_2[0], NUM_OUTPUTS);
    }
}
//:dsp1bend

//:dsp2start
static int32_t filter_states2[2][1*4];

static int32_t filter_coeffs2[1*5] = {
    0x10000000,  0x10000000,  0x10000000,  0x10000000,  0x10000000,
};

void dsp_thread2(chanend_t c_from1a, chanend_t c_from1b,
                 chanend_t c_todist) {     
    int from_1a[NUM_OUTPUTS];
    int from_1b[NUM_OUTPUTS];
    int for_usb[NUM_OUTPUTS];
    chan_out_buf_word(c_todist, &for_usb[0], NUM_OUTPUTS); // Sample -2
    chan_out_buf_word(c_todist, &for_usb[0], NUM_OUTPUTS); // Sample -1
    while(1) {
        // Pick up my chunk of data to work on
        chan_in_buf_word(c_from1a, &from_1a[0], NUM_OUTPUTS);
        chan_in_buf_word(c_from1b, &from_1b[0], NUM_OUTPUTS);


        for_usb[0] = dsp_filters_biquads((int32_t) from_1a[0],
                                        filter_coeffs2[0],
                                        filter_states2[0],
                                        1,
                                        28);
      
        for_usb[1] = dsp_filters_biquads((int32_t) from_1b[0],
                                        filter_coeffs2[1],
                                        filter_states2[1],
                                        1,
                                        28);
      
        // And deliver my answer back
        chan_out_buf_word(c_todist, &for_usb[0], NUM_OUTPUTS);
    }
}
//:dsp2end


//:diststart
void dsp_data_distributor(chanend_t c_usb, chanend_t c_to0, chanend_t c_from2) {     
    int for_usb [NUM_OUTPUTS];
    int from_usb[NUM_OUTPUTS];
    while(1) {
        // First deal with the USB side
        chan_in_buf_word( c_usb, &from_usb[0], NUM_OUTPUTS);
        chan_out_buf_word(c_usb, &for_usb[0],  NUM_OUTPUTS);
        // Now supply all data to both DSP tasks
        chan_out_buf_word(c_to0, &from_usb[0], NUM_OUTPUTS);
        // Now pick up data from DSP task 2
        chan_in_buf_word( c_from2, &for_usb[0], NUM_OUTPUTS);
    }
}
//:distend


#endif // DSP_PIPELINE
