
#ifndef _USERCODE_H_
#define _USERCODE_H_

#ifndef __STDC__
#include "xua_audio.h"
#include "xua_dsp.h"

[[combinable]]
void dsp_control(client dsp_ctrl_if i_dsp_ctrl);
//void dsp_control(client dsp_ctrl_if i_dsp_ctrl);

/* TODO move me? */
void dsp_buff(server audManage_if i_manAud);

/* Prototype for our custom genclock() task */
void genclock();

// TODO User NUM_DSP_CTRL_INTS

#ifndef XVSM 
#define USER_MAIN_CORES \
            on tile[1] : genclock(); \
            on tile[PDM_TILE].core[0]: user_pdm_process(i_mic_process); 
#else

#define USER_MAIN_CORES \
            on tile[1] : genclock(); \
            /*on tile[AUDIO_IO_TILE] : dsp_process(i_dsp, i_dsp_ctrl, 1); */\
            on tile[PDM_TILE] : dsp_buff(i_audMan); \
            on tile[PDM_TILE].core[0]: user_pdm_process(i_mic_process); \
            //on tile[PDM_TILE]: dsp_control(i_dsp_ctrl[0]);

#endif
#endif
#endif
