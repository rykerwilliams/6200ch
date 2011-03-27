/*
 * 6200ch - an external channel changer for Motorola DCT-6200 Tuner
 *
 * Copyright 2004,2005 by Stacey D. Son ( mythdev a son d org )
 * Copyright 2009 by Preston Crow (pc dash 6200 a crowcastle d net )
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <libavc1394/rom1394.h>
#include <libavc1394/avc1394.h>
#include <libraw1394/raw1394.h>
#include <sys/types.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <limits.h> //for LLONG_MAX
#include <getopt.h>
#include <unistd.h> // for usleep
#include <string.h>

// Vendor and Model IDs.
// NOTE: Some Models have more than one possible Vendor ID
// WARNING: Please update firewiredevice.cpp when adding to this list.

#define DCH3200_VENDOR_ID1 0x00001c11
#define DCH3200_VENDOR_ID2 0x00001cfb
#define DCH3200_VENDOR_ID3 0x00001fc4
#define DCH3200_VENDOR_ID4 0x000023a3
#define DCH3200_VENDOR_ID5 0x00001e5a
#define DCH3200_MODEL_ID1  0x0000d330
#define DCX3200_MODEL_ID1  0x0000f740
#define DCX3200_MODEL_ID2  0x0000fa07

#define DCX3432_VENDOR_ID1 0x000024a0
#define DCX3432_MODEL_ID1  0x0000ea05

#define DCH3416_VENDOR_ID1 0x00001e46
#define DCH3416_MODEL_ID1  0x0000b630

#define DCT3412_VENDOR_ID1 0x0000159a
#define DCT3412_MODEL_ID1  0x000034cb

#define DCT3416_VENDOR_ID1 0x000016b5
#define DCT3416_VENDOR_ID2 0x00001bdd
#define DCT3416_MODEL_ID1  0x0000346b
#define DCT3416_MODEL_ID2  0x0000b630

#define DCT5100_VENDOR_ID1 0x000017ee
#define DCT5100_MODEL_ID1  0x0000620a

#define DCT6200_VENDOR_ID1 0x00000ce5
#define DCT6200_VENDOR_ID2 0x00000e5c
#define DCT6200_VENDOR_ID3 0x00001225
#define DCT6200_VENDOR_ID4 0x00000f9f
#define DCT6200_VENDOR_ID5 0x00001180
#define DCT6200_VENDOR_ID6 0x000012c9
#define DCT6200_VENDOR_ID7 0x000011ae
#define DCT6200_VENDOR_ID8 0x0000152f
#define DCT6200_VENDOR_ID9 0x000014e8
#define DCT6200_VENDOR_ID10 0x000016b5
#define DCT6200_VENDOR_ID11 0x00001371
#define DCT6200_VENDOR_ID12 0x000019a6
#define DCT6200_VENDOR_ID13 0x00001aad
#define DCT6200_VENDOR_ID14 0x00000b06
#define DCT6200_VENDOR_ID15 0x0000195e
#define DCT6200_VENDOR_ID16 0x000010dc
#define DCT6200_SPEC_ID    0x00005068
#define DCT6200_SW_VERSION 0x00010101
#define DCT6200_MODEL_ID1  0x0000620a
#define DCT6200_MODEL_ID2  0x00006200

#define DCT6412_VENDOR_ID1 0x00000f9f
#define DCT6412_VENDOR_ID2 0x0000152f
#define DCT6412_MODEL_ID1  0x000064ca
#define DCT6412_MODEL_ID2  0x000064cb

#define DCT6416_VENDOR_ID1 0x000017ee
#define DCT6416_VENDOR_ID2 0x00001a66
#define DCT6416_MODEL_ID1  0x0000646b

#define QIP7100_VENDOR_ID1 0x00002374
#define QIP7100_VENDOR_ID2 0x000025f2
#define QIP7100_MODEL_ID1  0x00008100
#define QIP7100_MODEL_ID2  0x00000001
#define QIP7100_MODEL_ID3  0x00008500

#define QIP6200_VENDOR_ID1 0x0000211e
#define QIP6200_MODEL_ID1  0x00007100

#define MOT_UNKNOWN_VENDOR_ID1 0x04db
#define MOT_UNKNOWN_VENDOR_ID2 0x0406
#define MOT_UNKNOWN_VENDOR_ID3 0x0ce5
#define MOT_UNKNOWN_VENDOR_ID4 0x111a
#define MOT_UNKNOWN_VENDOR_ID5 0x1225
#define MOT_UNKNOWN_VENDOR_ID6 0x1404
#define MOT_UNKNOWN_VENDOR_ID7 0x1626
#define MOT_UNKNOWN_VENDOR_ID8 0x18c0
#define MOT_UNKNOWN_VENDOR_ID9 0x1ade
#define MOT_UNKNOWN_VENDOR_ID10 0x1cfb
#define MOT_UNKNOWN_VENDOR_ID11 0x2040
#define MOT_UNKNOWN_VENDOR_ID12 0x2180
#define MOT_UNKNOWN_VENDOR_ID13 0x2210
#define MOT_UNKNOWN_VENDOR_ID14 0x230b
#define MOT_UNKNOWN_VENDOR_ID15 0x2375
#define MOT_UNKNOWN_VENDOR_ID16 0x2395
#define MOT_UNKNOWN_VENDOR_ID17 0x23a2
#define MOT_UNKNOWN_VENDOR_ID18 0x23ed
#define MOT_UNKNOWN_VENDOR_ID19 0x23ee
#define MOT_UNKNOWN_VENDOR_ID20 0x23a0
#define MOT_UNKNOWN_VENDOR_ID21 0x23a1

#define PACE_VENDOR_ID1    0x00005094 /* 550 & 779 */
#define PACE_VENDOR_ID2    0x00005094 /* unknown */
#define PACE550_MODEL_ID1  0x00010551
#define PACE779_MODEL_ID1  0x00010755

#define AVC1394_6200_OPERAND_SET 0x20      /* 6200 subunit command operand */

#define CTL_CMD0 AVC1394_CTYPE_CONTROL | AVC1394_SUBUNIT_TYPE_PANEL | \
        AVC1394_SUBUNIT_ID_0 | AVC1394_PANEL_COMMAND_PASS_THROUGH | \
        AVC1394_6200_OPERAND_SET

#define STARTING_NODE 1  /* skip 1394 nodes to avoid error msgs */
#define STARTING_PORT 0
#define RETRY_COUNT_SLOW 1
#define RETRY_COUNT_FAST 0

/*
 * Definitions of key codes
 * taken from firewiredevice.h in the MythTV source code
 */
    typedef enum
    {
        kAVCPowerOn,
        kAVCPowerOff,
        kAVCPowerUnknown,
        kAVCPowerQueryFailed,
    } PowerState;

    // AVC commands
    typedef enum
    {
        kAVCControlCommand         = 0x00,
        kAVCStatusInquiryCommand   = 0x01,
        kAVCSpecificInquiryCommand = 0x02,
        kAVCNotifyCommand          = 0x03,
        kAVCGeneralInquiryCommand  = 0x04,

        kAVCNotImplementedStatus   = 0x08,
        kAVCAcceptedStatus         = 0x09,
        kAVCRejectedStatus         = 0x0a,
        kAVCInTransitionStatus     = 0x0b,
        kAVCImplementedStatus      = 0x0c,
        kAVCChangedStatus          = 0x0d,

        kAVCInterimStatus          = 0x0f,
        kAVCResponseImplemented    = 0x0c,
    } IEEE1394Command;

    // AVC unit addresses
    typedef enum
    {
        kAVCSubunitId0                = 0x00,
        kAVCSubunitId1                = 0x01,
        kAVCSubunitId2                = 0x02,
        kAVCSubunitId3                = 0x03,
        kAVCSubunitId4                = 0x04,
        kAVCSubunitIdExtended         = 0x05,
        kAVCSubunitIdIgnore           = 0x07,

        kAVCSubunitTypeVideoMonitor   = (0x00 << 3),
        kAVCSubunitTypeAudio          = (0x01 << 3),
        kAVCSubunitTypePrinter        = (0x02 << 3),
        kAVCSubunitTypeDiscRecorder   = (0x03 << 3),
        kAVCSubunitTypeTapeRecorder   = (0x04 << 3),
        kAVCSubunitTypeTuner          = (0x05 << 3),
        kAVCSubunitTypeCA             = (0x06 << 3),
        kAVCSubunitTypeVideoCamera    = (0x07 << 3),
        kAVCSubunitTypePanel          = (0x09 << 3),
        kAVCSubunitTypeBulletinBoard  = (0x0a << 3),
        kAVCSubunitTypeCameraStorage  = (0x0b << 3),
        kAVCSubunitTypeMusic          = (0x0c << 3),
        kAVCSubunitTypeVendorUnique   = (0x1c << 3),
        kAVCSubunitTypeExtended       = (0x1e << 3),
        kAVCSubunitTypeUnit           = (0x1f << 3),
    } IEEE1394UnitAddress;

    // AVC opcode
    typedef enum
    {
        // Unit
        kAVCUnitPlugInfoOpcode               = 0x02,
        kAVCUnitDigitalOutputOpcode          = 0x10,
        kAVCUnitDigitalInputOpcode           = 0x11,
        kAVCUnitChannelUsageOpcode           = 0x12,
        kAVCUnitOutputPlugSignalFormatOpcode = 0x18,
        kAVCUnitInputPlugSignalFormatOpcode  = 0x19,
        kAVCUnitConnectAVOpcode              = 0x20,
        kAVCUnitDisconnectAVOpcode           = 0x21,
        kAVCUnitConnectionsOpcode            = 0x22,
        kAVCUnitConnectOpcode                = 0x24,
        kAVCUnitDisconnectOpcode             = 0x25,
        kAVCUnitUnitInfoOpcode               = 0x30,
        kAVCUnitSubunitInfoOpcode            = 0x31,
        kAVCUnitSignalSourceOpcode           = 0x1a,
        kAVCUnitPowerOpcode                  = 0xb2,

        // Common Unit + Subunit
        kAVCCommonOpenDescriptorOpcode       = 0x08,
        kAVCCommonReadDescriptorOpcode       = 0x09,
        kAVCCommonWriteDescriptorOpcode      = 0x0A,
        kAVCCommonSearchDescriptorOpcode     = 0x0B,
        kAVCCommonObjectNumberSelectOpcode   = 0x0D,
        kAVCCommonPowerOpcode                = 0xB2,
        kAVCCommonReserveOpcode              = 0x01,
        kAVCCommonPlugInfoOpcode             = 0x02,
        kAVCCommonVendorDependentOpcode      = 0x00,

        // Panel
        kAVCPanelPassThrough                 = 0x7c,
    } IEEE1394Opcode;

    // AVC param 0
    typedef enum
    {
        kAVCPowerStateOn           = 0x70,
        kAVCPowerStateOff          = 0x60,
        kAVCPowerStateQuery        = 0x7f,
    } IEEE1394UnitPowerParam0;

    typedef enum
    {
        kAVCPanelKeySelect          = 0x00,
        kAVCPanelKeyUp              = 0x01,
        kAVCPanelKeyDown            = 0x02,
        kAVCPanelKeyLeft            = 0x03,
        kAVCPanelKeyRight           = 0x04,
        kAVCPanelKeyRightUp         = 0x05,
        kAVCPanelKeyRightDown       = 0x06,
        kAVCPanelKeyLeftUp          = 0x07,
        kAVCPanelKeyLeftDown        = 0x08,
        kAVCPanelKeyRootMenu        = 0x09,
        kAVCPanelKeySetupMenu       = 0x0A,
        kAVCPanelKeyContentsMenu    = 0x0B,
        kAVCPanelKeyFavoriteMenu    = 0x0C,
        kAVCPanelKeyExit            = 0x0D,

        kAVCPanelKey0               = 0x20,
        kAVCPanelKey1               = 0x21,
        kAVCPanelKey2               = 0x22,
        kAVCPanelKey3               = 0x23,
        kAVCPanelKey4               = 0x24,
        kAVCPanelKey5               = 0x25,
        kAVCPanelKey6               = 0x26,
        kAVCPanelKey7               = 0x27,
        kAVCPanelKey8               = 0x28,
        kAVCPanelKey9               = 0x29,
        kAVCPanelKeyDot             = 0x2A,
        kAVCPanelKeyEnter           = 0x2B,
        kAVCPanelKeyClear           = 0x2C,

        kAVCPanelKeyChannelUp       = 0x30,
        kAVCPanelKeyChannelDown     = 0x31,
        kAVCPanelKeyPreviousChannel = 0x32,
        kAVCPanelKeySoundSelect     = 0x33,
        kAVCPanelKeyInputSelect     = 0x34,
        kAVCPanelKeyDisplayInfo     = 0x35,
        kAVCPanelKeyHelp            = 0x36,
        kAVCPanelKeyPageUp          = 0x37,
        kAVCPanelKeyPageDown        = 0x38,

        kAVCPanelKeyPower           = 0x40,
        kAVCPanelKeyVolumeUp        = 0x41,
        kAVCPanelKeyVolumeDown      = 0x42,
        kAVCPanelKeyMute            = 0x43,
        kAVCPanelKeyPlay            = 0x44,
        kAVCPanelKeyStop            = 0x45,
        kAVCPanelKeyPause           = 0x46,
        kAVCPanelKeyRecord          = 0x47,
        kAVCPanelKeyRewind          = 0x48,
        kAVCPanelKeyFastForward     = 0x49,
        kAVCPanelKeyEject           = 0x4a,
        kAVCPanelKeyForward         = 0x4b,
        kAVCPanelKeyBackward        = 0x4c,

        kAVCPanelKeyAngle           = 0x50,
        kAVCPanelKeySubPicture      = 0x51,

        kAVCPanelKeyTuneFunction    = 0x67,

        kAVCPanelKeyPress           = 0x00,
        kAVCPanelKeyRelease         = 0x80,

    } IEEE1394PanelPassThroughParam0;

void send_ir_cmd(raw1394handle_t handle, int device, int verbose, int delaymsec, int key);
void set_chan_slow(raw1394handle_t handle, int device, int verbose, int delaymsec, int chn);
void set_chan_slow_four_digit(raw1394handle_t handle, int device, int verbose, int delaymsec, int chn);
void set_chan_fast(raw1394handle_t handle, int device, int verbose, int chn);
void set_power_fast(raw1394handle_t handle, int device, int verbose);
void set_menu_fast(raw1394handle_t handle, int device, int verbose, int chn);
void set_exit_fast(raw1394handle_t handle, int device, int verbose, int chn);
void query_power(raw1394handle_t handle, int device, int verbose);

void usage(){
   fprintf(stderr, "Usage: 6200ch [-v] [-s] [-w] [-m] [-e] [-4] [-q] "
           "[-n NODE] [-g GUID] [-p PORT] [-d DELAY] <channel_num|key_name>\n");
   fprintf(stderr, "-v        print additional verbose output\n");
   fprintf(stderr, "-s        use single packet method. Cannot be used with -4.\n");
   fprintf(stderr, "-w        toggle power state\n");
   fprintf(stderr, "-m        Sends \"Menu\" command.  Needed for some Motorola QIP series STB's.\n");
   fprintf(stderr, "-e        Sends \"Exit\" command.  Needed for some Motorola QIP series STB's.\n");
   fprintf(stderr, "-4        Enable 4 digit channel number support. Cannot be used with -s.\n");
   fprintf(stderr, "-d        inter-digit delay in ms (default 500)  Cannot be used with -s.\n");
   fprintf(stderr, "-n NODE   node to start device scanning on (default:%i)\n",
           STARTING_NODE);
   fprintf(stderr, "-p PORT   port/adapter to use              (default:%i)\n",
           STARTING_PORT);
   fprintf(stderr, "-g GUID   GUID to use, -n switch, if present, will be ignored.\n");
   fprintf(stderr, "-q        query current state\n");
   exit(1);
}

int main (int argc, char *argv[])
{
   rom1394_directory dir;
   int device = -1;
   int i;
   int verbose = 0;
   int single_packet = 0;
   int toggle_power = 0;
   int toggle_menu = 0;
   int toggle_exit = 0;
   int delaymsec=500;
   int query_state = 0;
   int bGUID=0;
   octlet_t cli_GUID=0LL;
   octlet_t node_GUID=0LL;
   int chn = 0;
   int key = -1;
   int use_four_digit = 0;

   /* some people experience crashes when starting on node 1 */
   int starting_node = STARTING_NODE;
   int starting_port = STARTING_PORT;
   int c;

   if (argc < 2)
      usage();

   opterr = 0;
   while ((c = getopt(argc, argv, "qvswme4g:n:p:d:")) != -1)
   {
       switch (c) {
       case 'v':
           verbose = 1;
           break;
       case 's':
           single_packet = 1;
           break;
       case 'w':
           toggle_power = 1;
           break;
       case 'm':
           toggle_menu = 1;
           break;
       case 'e':
           toggle_exit = 1;
           break;
       case '4':
           use_four_digit = 1;
           break;
       case 'n':
           starting_node = atoi(optarg);
           break;
       case 'd':
           delaymsec = atoi(optarg);
           break;
       case 'g':
           bGUID=1;
           starting_node=0;
           cli_GUID = (octlet_t)strtoll(optarg, (char **)NULL, 16);
           break;
       case 'p':
           starting_port = atoi(optarg);
           break;
       case 'q':
      query_state = 1;
      break;
       default:
           fprintf(stderr, "incorrect command line arguments\n");
           usage();
       }
   }

   {
   /* We cannot use single packet with 4 digits */
       if (single_packet && use_four_digit)
           usage();
   }
   /* print out usage message if not enough arguments */
   if (optind == argc-1)
   {
   /* the last argument is the channel number */
       chn = atoi(argv[optind]);

       if ( chn ) ;
       /* grep 'kAVCPanelKey[^0-9].*= 0x' 6200ch.c | sed -e 'sX=.*XX' -e 'sX XXg' -e 'sX\(.*Key\)\(.*\)X       else if ( 0 == strcasecmp(argv[optind],"\2" ) ) key=\1\2;X' */
       else if ( 0 == strcasecmp(argv[optind],"Select" ) ) key=kAVCPanelKeySelect;
       else if ( 0 == strcasecmp(argv[optind],"Up" ) ) key=kAVCPanelKeyUp;
       else if ( 0 == strcasecmp(argv[optind],"Down" ) ) key=kAVCPanelKeyDown;
       else if ( 0 == strcasecmp(argv[optind],"Left" ) ) key=kAVCPanelKeyLeft;
       else if ( 0 == strcasecmp(argv[optind],"Right" ) ) key=kAVCPanelKeyRight;
       else if ( 0 == strcasecmp(argv[optind],"RightUp" ) ) key=kAVCPanelKeyRightUp;
       else if ( 0 == strcasecmp(argv[optind],"RightDown" ) ) key=kAVCPanelKeyRightDown;
       else if ( 0 == strcasecmp(argv[optind],"LeftUp" ) ) key=kAVCPanelKeyLeftUp;
       else if ( 0 == strcasecmp(argv[optind],"LeftDown" ) ) key=kAVCPanelKeyLeftDown;
       else if ( 0 == strcasecmp(argv[optind],"RootMenu" ) ) key=kAVCPanelKeyRootMenu;
       else if ( 0 == strcasecmp(argv[optind],"SetupMenu" ) ) key=kAVCPanelKeySetupMenu;
       else if ( 0 == strcasecmp(argv[optind],"ContentsMenu" ) ) key=kAVCPanelKeyContentsMenu;
       else if ( 0 == strcasecmp(argv[optind],"FavoriteMenu" ) ) key=kAVCPanelKeyFavoriteMenu;
       else if ( 0 == strcasecmp(argv[optind],"Exit" ) ) key=kAVCPanelKeyExit;
       else if ( 0 == strcasecmp(argv[optind],"Dot" ) ) key=kAVCPanelKeyDot;
       else if ( 0 == strcasecmp(argv[optind],"Enter" ) ) key=kAVCPanelKeyEnter;
       else if ( 0 == strcasecmp(argv[optind],"Clear" ) ) key=kAVCPanelKeyClear;
       else if ( 0 == strcasecmp(argv[optind],"ChannelUp" ) ) key=kAVCPanelKeyChannelUp;
       else if ( 0 == strcasecmp(argv[optind],"ChannelDown" ) ) key=kAVCPanelKeyChannelDown;
       else if ( 0 == strcasecmp(argv[optind],"PreviousChannel" ) ) key=kAVCPanelKeyPreviousChannel;
       else if ( 0 == strcasecmp(argv[optind],"SoundSelect" ) ) key=kAVCPanelKeySoundSelect;
       else if ( 0 == strcasecmp(argv[optind],"InputSelect" ) ) key=kAVCPanelKeyInputSelect;
       else if ( 0 == strcasecmp(argv[optind],"DisplayInfo" ) ) key=kAVCPanelKeyDisplayInfo;
       else if ( 0 == strcasecmp(argv[optind],"Help" ) ) key=kAVCPanelKeyHelp;
       else if ( 0 == strcasecmp(argv[optind],"PageUp" ) ) key=kAVCPanelKeyPageUp;
       else if ( 0 == strcasecmp(argv[optind],"PageDown" ) ) key=kAVCPanelKeyPageDown;
       else if ( 0 == strcasecmp(argv[optind],"Power" ) ) key=kAVCPanelKeyPower;
       else if ( 0 == strcasecmp(argv[optind],"VolumeUp" ) ) key=kAVCPanelKeyVolumeUp;
       else if ( 0 == strcasecmp(argv[optind],"VolumeDown" ) ) key=kAVCPanelKeyVolumeDown;
       else if ( 0 == strcasecmp(argv[optind],"Mute" ) ) key=kAVCPanelKeyMute;
       else if ( 0 == strcasecmp(argv[optind],"Play" ) ) key=kAVCPanelKeyPlay;
       else if ( 0 == strcasecmp(argv[optind],"Stop" ) ) key=kAVCPanelKeyStop;
       else if ( 0 == strcasecmp(argv[optind],"Pause" ) ) key=kAVCPanelKeyPause;
       else if ( 0 == strcasecmp(argv[optind],"Record" ) ) key=kAVCPanelKeyRecord;
       else if ( 0 == strcasecmp(argv[optind],"Rewind" ) ) key=kAVCPanelKeyRewind;
       else if ( 0 == strcasecmp(argv[optind],"FastForward" ) ) key=kAVCPanelKeyFastForward;
       else if ( 0 == strcasecmp(argv[optind],"Eject" ) ) key=kAVCPanelKeyEject;
       else if ( 0 == strcasecmp(argv[optind],"Forward" ) ) key=kAVCPanelKeyForward;
       else if ( 0 == strcasecmp(argv[optind],"Backward" ) ) key=kAVCPanelKeyBackward;
       else if ( 0 == strcasecmp(argv[optind],"Angle" ) ) key=kAVCPanelKeyAngle;
       else if ( 0 == strcasecmp(argv[optind],"SubPicture" ) ) key=kAVCPanelKeySubPicture;
       else if ( 0 == strcasecmp(argv[optind],"TuneFunction" ) ) key=kAVCPanelKeyTuneFunction;
       else if ( 0 == strcasecmp(argv[optind],"Press" ) ) key=kAVCPanelKeyPress;
       else if ( 0 == strcasecmp(argv[optind],"Release" ) ) key=kAVCPanelKeyRelease;
   }
   else if (!toggle_power && !query_state)
   {
       usage();
   }

#ifdef RAW1394_V_0_8
   raw1394handle_t handle = raw1394_get_handle();
#else
   raw1394handle_t handle = raw1394_new_handle();
#endif

   if (!handle) {
      if (!errno) {
         fprintf(stderr, "Not Compatible!\n");
      } else {
         perror("Couldn't get 1394 handle");
         fprintf(stderr, "Is ieee1394, driver, and raw1394 loaded?\n");
      }
      exit(1);
   }

   if (raw1394_set_port(handle, starting_port) < 0) {
      perror("couldn't set port");
      raw1394_destroy_handle(handle);
      exit(1);
   }

   if (verbose)
       printf("starting with node: %d\n", starting_node);

   int nc = raw1394_get_nodecount(handle);
   if (bGUID!=0) {
      if (cli_GUID==0LL || cli_GUID==LLONG_MAX || cli_GUID==LLONG_MIN) {
          fprintf(stderr, "error parsing GUID command line parameter\n");
          exit(1);
      }
   }
   for (i=starting_node; i < nc; ++i) {
      if (bGUID!=0) {
         node_GUID=rom1394_get_guid(handle, i);
#ifdef DEBUG
         printf("node=%d, node_GUID=%LX, cli_GUID=%LX\n", i, node_GUID, cli_GUID);
#endif
         if (cli_GUID!=node_GUID) {
             continue;
         }
      }

      if (rom1394_get_directory(handle, i, &dir) < 0) {
         fprintf(stderr,"error reading config rom directory for node %d\n", i);
         raw1394_destroy_handle(handle);
         exit(1);
      }

      if (verbose)
         printf("node %d: vendor_id = 0x%08x model_id = 0x%08x\n",
                 i, dir.vendor_id, dir.model_id);

      // WARNING: Please update firewiredevice.cpp when adding to this list.
      if ( ((dir.vendor_id == DCH3200_VENDOR_ID1) ||
            (dir.vendor_id == DCH3200_VENDOR_ID2) ||
            (dir.vendor_id == DCH3200_VENDOR_ID3) ||
            (dir.vendor_id == DCH3200_VENDOR_ID4) ||
            (dir.vendor_id == DCH3200_VENDOR_ID5) ||
            (dir.vendor_id == DCH3416_VENDOR_ID1) ||
            (dir.vendor_id == DCT3412_VENDOR_ID1) ||
            (dir.vendor_id == DCT3416_VENDOR_ID1) ||
            (dir.vendor_id == DCT3416_VENDOR_ID2) ||
            (dir.vendor_id == DCT5100_VENDOR_ID1) ||
            (dir.vendor_id == DCT6200_VENDOR_ID1) ||
            (dir.vendor_id == DCT6200_VENDOR_ID2) ||
            (dir.vendor_id == DCT6200_VENDOR_ID3) ||
            (dir.vendor_id == DCT6200_VENDOR_ID4) ||
            (dir.vendor_id == DCT6200_VENDOR_ID5) ||
            (dir.vendor_id == DCT6200_VENDOR_ID6) ||
            (dir.vendor_id == DCT6200_VENDOR_ID7) ||
            (dir.vendor_id == DCT6200_VENDOR_ID8) ||
            (dir.vendor_id == DCT6200_VENDOR_ID9) ||
            (dir.vendor_id == DCT6200_VENDOR_ID10) ||
            (dir.vendor_id == DCT6200_VENDOR_ID11) ||
            (dir.vendor_id == DCT6200_VENDOR_ID12) ||
            (dir.vendor_id == DCT6200_VENDOR_ID13) ||
            (dir.vendor_id == DCT6200_VENDOR_ID14) ||
            (dir.vendor_id == DCT6200_VENDOR_ID15) ||
            (dir.vendor_id == DCT6200_VENDOR_ID16) ||
            (dir.vendor_id == DCT6412_VENDOR_ID1) ||
            (dir.vendor_id == DCT6412_VENDOR_ID2) ||
            (dir.vendor_id == DCT6416_VENDOR_ID1) ||
            (dir.vendor_id == DCT6416_VENDOR_ID2) ||
            (dir.vendor_id == DCX3432_VENDOR_ID1) ||
            (dir.vendor_id == QIP7100_VENDOR_ID1) ||
            (dir.vendor_id == QIP7100_VENDOR_ID2) ||
            (dir.vendor_id == QIP6200_VENDOR_ID1) ||
            (dir.vendor_id == MOT_UNKNOWN_VENDOR_ID1) ||
            (dir.vendor_id == MOT_UNKNOWN_VENDOR_ID2) ||
            (dir.vendor_id == MOT_UNKNOWN_VENDOR_ID3) ||
            (dir.vendor_id == MOT_UNKNOWN_VENDOR_ID4) ||
            (dir.vendor_id == MOT_UNKNOWN_VENDOR_ID5) ||
            (dir.vendor_id == MOT_UNKNOWN_VENDOR_ID6) ||
            (dir.vendor_id == MOT_UNKNOWN_VENDOR_ID7) ||
            (dir.vendor_id == MOT_UNKNOWN_VENDOR_ID8) ||
            (dir.vendor_id == MOT_UNKNOWN_VENDOR_ID9) ||
            (dir.vendor_id == MOT_UNKNOWN_VENDOR_ID10) ||
            (dir.vendor_id == MOT_UNKNOWN_VENDOR_ID11) ||
            (dir.vendor_id == MOT_UNKNOWN_VENDOR_ID12) ||
            (dir.vendor_id == MOT_UNKNOWN_VENDOR_ID13) ||
            (dir.vendor_id == MOT_UNKNOWN_VENDOR_ID14) ||
            (dir.vendor_id == MOT_UNKNOWN_VENDOR_ID15) ||
            (dir.vendor_id == MOT_UNKNOWN_VENDOR_ID16) ||
            (dir.vendor_id == MOT_UNKNOWN_VENDOR_ID17) ||
            (dir.vendor_id == MOT_UNKNOWN_VENDOR_ID18) ||
            (dir.vendor_id == MOT_UNKNOWN_VENDOR_ID19) ||
            (dir.vendor_id == MOT_UNKNOWN_VENDOR_ID20) ||
            (dir.vendor_id == MOT_UNKNOWN_VENDOR_ID21) ||
            (dir.vendor_id == PACE_VENDOR_ID1) ||
            (dir.vendor_id == PACE_VENDOR_ID2)) &&
           ((dir.model_id == DCH3200_MODEL_ID1) ||
            (dir.model_id == DCX3200_MODEL_ID1) ||
            (dir.model_id == DCX3200_MODEL_ID2) ||
            (dir.model_id == DCX3432_MODEL_ID1) ||
            (dir.model_id == DCH3416_MODEL_ID1) ||
            (dir.model_id == DCT3412_MODEL_ID1) ||
            (dir.model_id == DCT3416_MODEL_ID1) ||
            (dir.model_id == DCT3416_MODEL_ID2) ||
            (dir.model_id == DCT5100_MODEL_ID1) ||
            (dir.model_id == DCT6200_MODEL_ID1) ||
            (dir.model_id == DCT6200_MODEL_ID2) ||
            (dir.model_id == DCT6412_MODEL_ID1) ||
            (dir.model_id == DCT6412_MODEL_ID2) ||
            (dir.model_id == DCT6416_MODEL_ID1) ||
            (dir.model_id == QIP7100_MODEL_ID1) ||
            (dir.model_id == QIP7100_MODEL_ID2) ||
            (dir.model_id == QIP7100_MODEL_ID3) ||
            (dir.model_id == QIP6200_MODEL_ID1) ||
            (dir.model_id == PACE550_MODEL_ID1) ||
            (dir.model_id == PACE779_MODEL_ID1)) )
      {
            if (dir.unit_spec_id != DCT6200_SPEC_ID)
               fprintf(stderr, "Warning: Unit Spec ID different.\n");
            if (dir.unit_sw_version != DCT6200_SW_VERSION)
               fprintf(stderr, "Warning: Unit Software Version different.\n");
            device = i;
            break;
      }
   }

   if (device == -1) {
        fprintf(stderr, "Could not find Motorola DCT-6200 on the 1394 bus.\n");
        raw1394_destroy_handle(handle);
        exit(1);
   }

   if (toggle_power)
       set_power_fast(handle, device, verbose);

   if (toggle_menu)
       set_menu_fast(handle, device, verbose, chn);

   if (toggle_exit)
       set_exit_fast(handle, device, verbose, chn);

   if (query_state)
       query_power(handle, device, verbose);

   if (chn)
   {
       if (single_packet)
           set_chan_fast(handle, device, verbose, chn);
       else
       if (use_four_digit)
           set_chan_slow_four_digit(handle, device, verbose, delaymsec, chn);
       else
           set_chan_slow(handle, device, verbose, delaymsec, chn);
   }
   if (key != -1) /* Send key by name */
       send_ir_cmd(handle, device, verbose, delaymsec, key);

   raw1394_destroy_handle(handle);
   exit(0);
}

/*
 * send_ir_cmd()
 *
 * Send one command as if the button were pressed on the remote
 */
void send_ir_cmd(raw1394handle_t handle, int device, int verbose, int delaymsec, int key)
{
   quadlet_t cmd[2];

   cmd[0] = (CTL_CMD0) & ~0xff;
   cmd[1] = 0x0;
   cmd[0] |= key;

   if (verbose)
      printf("AV/C Command: Op=0x%08X\n", cmd[0]);

   avc1394_transaction_block(handle, device, cmd, 2, RETRY_COUNT_SLOW);
   usleep(delaymsec *1000);  // small delay for button to register
}

void set_chan_slow(raw1394handle_t handle, int device, int verbose, int delaymsec, int chn)
{
   int i;
   int dig[3];
   quadlet_t cmd[2];

   dig[2] = (chn % 10);
   dig[1] = (chn % 100)  / 10;
   dig[0] = (chn % 1000) / 100;

   send_ir_cmd(handle, device, verbose, delaymsec, kAVCPanelKeyExit ); /* Exit any menus first */

   if (verbose)
      printf("AV/C Command: %d%d%d = Op1=0x%08X Op2=0x%08X Op3=0x%08X\n",
            dig[0], dig[1], dig[2],
            CTL_CMD0 | dig[0], CTL_CMD0 | dig[1], CTL_CMD0 | dig[2]);

   for (i=0; i<3; i++) {
      cmd[0] = CTL_CMD0 | dig[i];
      cmd[1] = 0x0;

      avc1394_transaction_block(handle, device, cmd, 2, RETRY_COUNT_SLOW);
      usleep(delaymsec *1000); // small delay for button to register
   }
}

// Same as set_chan_slow(), but sends 4 digits instead of 3 (for quicker channel changes on QIP6200-2 / QIP7100-1)
void set_chan_slow_four_digit(raw1394handle_t handle, int device, int verbose, int delaymsec, int chn)
{
   int i;
   int dig[4];
   quadlet_t cmd[2];

   if (verbose)
       printf("chn: %d\n", chn);

   dig[3] = (chn % 10);
   dig[2] = (chn % 100) / 10;
   dig[1] = (chn % 1000) / 100;
   dig[0] = (chn % 10000) / 1000;

   if (verbose)
      printf("AV/C Command: %d%d%d%d = Op1=0x%08X Op2=0x%08X Op3=0x%08X Op4=0x%08X\n",
            dig[0], dig[1], dig[2], dig[3],
            CTL_CMD0 | dig[0], CTL_CMD0 | dig[1], CTL_CMD0 | dig[2], CTL_CMD0 | dig[3]);

   for (i=0; i<4; i++) {
      if (verbose)
          printf("Sending digit %d\n", dig[i]);
      cmd[0] = CTL_CMD0 | dig[i];
      cmd[1] = 0x0;

      avc1394_transaction_block(handle, device, cmd, 2, RETRY_COUNT_SLOW);
      usleep(delaymsec *1000); // small delay for button to register
   }
}

void set_chan_fast(raw1394handle_t handle, int device, int verbose, int chn)
{
    quadlet_t cmd[3];

    send_ir_cmd(handle, device, verbose, 500000, kAVCPanelKeyExit ); /* Exit any menus first */

    cmd[0] = CTL_CMD0 | 0x67;
    cmd[1] = (0x04 << 24) | (chn << 8) | 0x000000FF;
    cmd[2] = 0xFF << 24;

    if (verbose)
        printf("AV/C command for channel %d = 0x%08X %08X %08X\n",
               chn, cmd[0], cmd[1], cmd[2]);

    avc1394_transaction_block(handle, device, cmd, 3, RETRY_COUNT_FAST);
}

void set_power_fast(raw1394handle_t handle, int device, int verbose)
{
    quadlet_t cmd[2];
    quadlet_t *response;

    cmd[0] = ((CTL_CMD0) & 0xffffff00) | AVC1394_PANEL_OPERATION_POWER;
    cmd[1] = 0;

    response = avc1394_transaction_block(
        handle, device, cmd, 2, RETRY_COUNT_FAST);
    usleep(500000);  // small delay for button to register

    if (verbose)
    {
        printf("AV/C command for power = 0x%08X\n", cmd[0]);
        if (NULL != response)
          printf("  (%x) 0x%x\n", (*response) >> 24, *response);
        else
          printf("  No response\n");
    }
}

void query_power(raw1394handle_t handle, int device, int verbose)
{
    quadlet_t cmd[2];
    quadlet_t *response;

    cmd[0] = (kAVCStatusInquiryCommand);
    cmd[0] <<= 8;
    cmd[0] |= (kAVCSubunitTypeUnit | kAVCSubunitIdIgnore);
    cmd[0] <<= 8;
    cmd[0] |= (kAVCUnitPowerOpcode);
    cmd[0] <<= 8;
    cmd[0] |= (kAVCPowerStateQuery);
    cmd[1] = 0;

    response = avc1394_transaction_block(
        handle, device, cmd, 2, RETRY_COUNT_FAST);

    if (verbose)
    {
        printf("AV/C command for power query = 0x%08X\n", cmd[0]);
        if (NULL != response)
          printf("  (%x) 0x%x\n", (*response) >> 24, *response);
        else
          printf("  No response\n");
    }
    switch ((*response)&0xff) {
    case kAVCPowerStateOn:
       printf("Power is on\n");
       break;
    case kAVCPowerStateOff:
       printf("Power is off\n");
       break;
    default:
       printf("Power is unknown\n");
       break;
    }
}

void set_menu_fast(raw1394handle_t handle, int device, int verbose, int chn)
{
    quadlet_t cmd[2];
    quadlet_t *response;

    cmd[0] = ((CTL_CMD0) & 0xffffff00) | AVC1394_PANEL_OPERATION_ROOT_MENU;
    cmd[1] = 0;

    response = avc1394_transaction_block(
        handle, device, cmd, 2, RETRY_COUNT_FAST);
    usleep(1000000);  // small delay for button to register

    if (verbose)
    {
        printf("AV/C command for menu = 0x%08X\n", cmd[0]);
        if (NULL != response)
          printf("  (%x) 0x%x\n", (*response) >> 24, *response);
        else
          printf("  No response\n");
    }
}

void set_exit_fast(raw1394handle_t handle, int device, int verbose, int chn)
{
    quadlet_t cmd[2];
    quadlet_t *response;

    cmd[0] = ((CTL_CMD0) & 0xffffff00) | AVC1394_PANEL_OPERATION_EXIT;
    cmd[1] = 0;

    response = avc1394_transaction_block(
        handle, device, cmd, 2, RETRY_COUNT_FAST);
    usleep(1000000);  // small delay for button to register

    if (verbose)
    {
        printf("AV/C command for exit = 0x%08X\n", cmd[0]);
        if (NULL != response)
          printf("  (%x) 0x%x\n", (*response) >> 24, *response);
        else
          printf("  No response\n");
    }
}
