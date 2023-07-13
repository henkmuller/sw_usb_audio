#ifndef USER_MAIN_H
#define USER_MAIN_H

#ifdef __XC__

#include "i2c.h"
#include <print.h>
#include <xs1.h>
#include <platform.h>

extern unsafe client interface i2c_master_if i_i2c_client;
extern void interface_saver(client interface i2c_master_if i);
extern void ctrlPort();
#ifdef DSP_MULTI_THREAD
extern void UserBufferManagementSetChan(chanend t, chanend u);
extern void dsp_main1(chanend t);
extern void dsp_main2(chanend t);
#endif

#ifdef DSP_SINGLE_THREAD
extern void UserBufferManagementSetChan(chanend t);
extern void dsp_main(chanend t);
#endif

#ifdef DSP_PIPELINE
extern void UserBufferManagementSetChan(chanend t);
extern void dsp_main(chanend t);
#endif

/* I2C interface ports */
extern port p_scl;
extern port p_sda;

#ifdef DSP_USB_THREAD

//:usbstart
#define USER_MAIN_DECLARATIONS  \
    interface i2c_master_if i2c[1];

#define USER_MAIN_CORES \
    on tile[0]: {                                         \
        ctrlPort();                                                     \
        i2c_master(i2c, 1, p_scl, p_sda, 100);                          \
    }                                                                   \
    on tile[1]: {                                                       \
        unsafe                                                          \
        {                                                               \
            i_i2c_client = i2c[0];                                      \
        }                                                         \
    }
//:usbend

#endif


#ifdef DSP_SINGLE_THREAD

//:singlestart
#define USER_MAIN_DECLARATIONS  \
    chan c_data_transport;      \
    interface i2c_master_if i2c[1];

#define USER_MAIN_CORES \
    on tile[1]: {                                 \
        dsp_main(c_data_transport);                       \
    }                                                     \
    on tile[0]: {                                         \
        ctrlPort();                                                     \
        i2c_master(i2c, 1, p_scl, p_sda, 100);                          \
    }                                                                   \
    on tile[1]: {                                                       \
        UserBufferManagementSetChan(c_data_transport);    \
        unsafe                                                          \
        {                                                               \
            i_i2c_client = i2c[0];                                      \
        }                                                         \
    }
//:singleend

#endif

#ifdef DSP_MULTI_THREAD

//:multistart
#define USER_MAIN_DECLARATIONS                     \
    chan c1, c2;                                   \
    interface i2c_master_if i2c[1];

#define USER_MAIN_CORES \
    on tile[1]: {                                 \
        dsp_main1(c1);                                    \
    }                                                     \
    on tile[1]: {                                 \
        dsp_main2(c2);                                    \
    }                                                     \
    on tile[0]: {                                         \
        ctrlPort();                                       \
        i2c_master(i2c, 1, p_scl, p_sda, 100);            \
    }                                                     \
    on tile[1]: {                                         \
        UserBufferManagementSetChan(c1, c2);              \
        unsafe                                            \
        {                                                 \
            i_i2c_client = i2c[0];                        \
        }                                                 \
    }
//:multiend


#endif

#ifdef DSP_PIPELINE

#define USER_MAIN_DECLARATIONS                     \
    chan c;                                        \
    interface i2c_master_if i2c[1];

#define USER_MAIN_CORES \
    on tile[1]: {                                         \
        dsp_main(c);                                      \
    }                                                     \
    on tile[0]: {                                         \
        ctrlPort();                                       \
        i2c_master(i2c, 1, p_scl, p_sda, 100);            \
    }                                                     \
    on tile[1]: {                                         \
        UserBufferManagementSetChan(c);                   \
        unsafe                                            \
        {                                                 \
            i_i2c_client = i2c[0];                        \
        }                                                 \
    }


#endif

#endif

#endif
