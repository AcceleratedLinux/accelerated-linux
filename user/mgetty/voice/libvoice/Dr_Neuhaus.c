/*
 * Dr_Neuhaus.c
 *
 * This file contains the Dr. Neuhaus Cybermod specific hardware stuff.
 *
 * $Id: Dr_Neuhaus.c,v 1.10 2006/09/26 17:17:55 gert Exp $
 *
 */

#include "../include/voice.h"

static int Dr_Neuhaus_init (void)
     {
     char buffer[VOICE_BUF_LEN];

     reset_watchdog();
     voice_modem_state = INITIALIZING;
     lprintf(L_MESG, "initializing Dr. Neuhaus voice modem");

     /*
      * AT+VSD=x,y - Set silence threshold and duration.
      */

     sprintf(buffer, "AT+VSD=%d,%d", (int)cvd.rec_silence_threshold.d.i *
      10 / 100 + 123, (int)cvd.rec_silence_len.d.i);

     if (voice_command(buffer, "OK") != VMA_USER_1)
          lprintf(L_WARN, "setting recording preferences didn't work");

     /*
      * AT+VGT - Set the transmit gain for voice samples.
      */

     if (cvd.transmit_gain.d.i == -1)
          cvd.transmit_gain.d.i = 100;

     sprintf(buffer, "AT+VGT=%d", (int)cvd.transmit_gain.d.i * 10 / 100 + 123);

     if (voice_command(buffer, "OK") != VMA_USER_1)
          lprintf(L_WARN, "setting transmit gain didn't work");

     /*
      * AT+VGR - Set receive gain for voice samples.
      */

     if (cvd.receive_gain.d.i == -1)
          cvd.receive_gain.d.i = 40;

     sprintf(buffer, "AT+VGR=%d", (int)cvd.receive_gain.d.i * 10 / 100 + 123);

     if (voice_command(buffer, "OK") != VMA_USER_1)
          lprintf(L_WARN, "setting receive gain didn't work");

     if (voice_command("AT+VIT=0", "OK") != VMA_USER_1)
          lprintf(L_WARN, "can't deactivate inactivity timer");

     if (voice_command("AT+VPR=0", "OK") != VMA_USER_1)
          lprintf(L_WARN, "can't select autobauding");

     if (voice_command("AT+VLS=0", "OK") != VMA_USER_1)
          lprintf(L_WARN, "can't deselect all input/output devices");

     voice_modem_state = IDLE;
     return(OK);
     }

static int Dr_Neuhaus_set_compression (p_int *compression, p_int *speed, int *bits)
     {
     reset_watchdog();

     if (*compression == 0)
          *compression = 129;

     if (*speed == 0)
          *speed = 8000;

     if (*speed != 8000)
          {
          lprintf(L_WARN, "%s: Illegal sample rate (%d)", voice_modem_name,
           *speed);
          return(FAIL);
          };

     switch (*compression)
          {
          case 128:
               *bits = 8;

               if (voice_command("AT+VSM=128", "OK") != VMA_USER_1)
                    return(FAIL);

               break;
          case 129:
               *bits = 2;

               if (voice_command("AT+VSM=129", "OK") != VMA_USER_1)
                    return(FAIL);

               break;
          case 130:
               *bits = 3;

               if (voice_command("AT+VSM=130", "OK") != VMA_USER_1)
                    return(FAIL);

               break;
          default:
               lprintf(L_WARN, "%s: Illegal voice compression method (%d)",
                voice_modem_name, *compression);
               return(FAIL);
          };

     return(OK);
     }

static int Dr_Neuhaus_set_device (int device)
     {
     reset_watchdog();

     switch (device)
          {
          case NO_DEVICE:
               voice_command("AT+VLS=0", "OK");
               return(OK);
          case DIALUP_LINE:
               voice_command("AT+VLS=2", "OK");
               return(OK);
          case INTERNAL_SPEAKER:
               voice_command("AT+VLS=4", "OK");
               return(OK);
          case INTERNAL_MICROPHONE:
	       /* The Smarty has an INTERNAL_MICROPHONE.
		* -- Raoul Boenisch <Raoul.Boenisch@uni-essen.de>
		*/
               voice_command("AT+VLS=14", "OK");
               return(OK);

          };

     lprintf(L_WARN, "%s: Unknown output device (%d)", voice_modem_name,
      device);
     return(FAIL);
     }

voice_modem_struct Dr_Neuhaus =
     {
     "Dr. Neuhaus Cybermod",
     "Dr. Neuhaus",
     (char *) IS_101_pick_phone_cmnd,
     (char *) IS_101_pick_phone_answr,
     (char *) IS_101_beep_cmnd,
     (char *) IS_101_beep_answr,
              IS_101_beep_timeunit,
     (char *) IS_101_hardflow_cmnd,
     (char *) IS_101_hardflow_answr,
     (char *) IS_101_softflow_cmnd,
     (char *) IS_101_softflow_answr,
     (char *) IS_101_start_play_cmnd,
     (char *) IS_101_start_play_answer,
     (char *) IS_101_reset_play_cmnd,
     (char *) IS_101_intr_play_cmnd,
     (char *) IS_101_intr_play_answr,
     (char *) IS_101_stop_play_cmnd,
     (char *) IS_101_stop_play_answr,
     (char *) IS_101_start_rec_cmnd,
     (char *) IS_101_start_rec_answr,
     (char *) IS_101_stop_rec_cmnd,
     (char *) IS_101_stop_rec_answr,
     (char *) IS_101_switch_mode_cmnd,
     (char *) IS_101_switch_mode_answr,
     (char *) IS_101_ask_mode_cmnd,
     (char *) IS_101_ask_mode_answr,
     (char *) IS_101_voice_mode_id,
     (char *) IS_101_play_dtmf_cmd,
     (char *) IS_101_play_dtmf_extra,
     (char *) IS_101_play_dtmf_answr,
     // juergen.kosel@gmx.de : voice-duplex-patch start
     NULL,  /* (char *) V253modem_start_duplex_voice_cmnd, */
     NULL,  /* (char *) V253modemstart_duplex_voice_answr, */
     NULL,  /* (char *) V253modem_stop_duplex_voice_cmnd , */
     NULL,  /* (char *) V253modem_stop_duplex_voice_answr, */
     // juergen.kosel@gmx.de : voice-duplex-patch end

     &IS_101_answer_phone,
     &IS_101_beep,
     &IS_101_dial,
     &IS_101_handle_dle,
     &Dr_Neuhaus_init,
     &IS_101_message_light_off,
     &IS_101_message_light_on,
     &IS_101_start_play_file,
     &IS_101_reset_play_file,
     &IS_101_stop_play_file,
     &IS_101_play_file,
     &IS_101_record_file,
     &Dr_Neuhaus_set_compression,
     &Dr_Neuhaus_set_device,
     &IS_101_stop_dialing,
     &IS_101_stop_playing,
     &IS_101_stop_recording,
     &IS_101_stop_waiting,
     &IS_101_switch_to_data_fax,
     &IS_101_voice_mode_off,
     &IS_101_voice_mode_on,
     &IS_101_wait,
     &IS_101_play_dtmf,
     &IS_101_check_rmd_adequation,
     // juergen.kosel@gmx.de : voice-duplex-patch start
     &IS_101_handle_duplex_voice,
     NULL, /* since there is no way to enter duplex voice state */
     // juergen.kosel@gmx.de : voice-duplex-patch end
     0
     };
