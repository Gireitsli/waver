/*
#==========================================
#
#      ___           ___           ___     
#     /\__\         /\  \         /\__\    
#    /:/ _/_       /::\  \       /:/  /    
#   /:/ /\__\     /:/\:\  \     /:/  /     
#  /:/ /:/ _/_   /::\~\:\  \   /:/__/  ___ 
# /:/_/:/ /\__\ /:/\:\ \:\__\  |:|  | /\__\
# \:\/:/ /:/  / \/__\:\/:/  /  |:|  |/:/  /
#  \::/_/:/  /       \::/  /   |:|__/:/  / 
#   \:\/:/  /        /:/  /     \::::/__/  
#    \::/  /        /:/  /       ~~~~      
#     \/__/         \/__/                  
#
# WAV creator
# =========================================
# File:      waver.h
#
# Author:    Prosper Leibundgut
#            <pl|at|vqe.ch>
#
# Thanks to: Heikki Hannikainen
#            <hessu|at|hes.iki.fi>
#            For sharing the sources of his
#            "bchunk", which served
#            important informations
#            for this implementation.
#
# Date:      09/2015
#
#==========================================
*/
#ifndef WAVER_H_
#define WAVER_H_

/* we need to define gnu source to make use 
 * of the functions strcasestr and syncfs 
 * since these functions are nonstandard 
 * extensions.
 *
 * must be defined before any include.
 *
 * SUCH A MICKEY MOUSE THING!!! ...
 *
 * if it causes too many problems on other platforms
 * than GNU/Linux, we replace the functions 
 * "strcasestr" and "syncfs" in later releases.
 */
#define _GNU_SOURCE

#include <stdint.h>

/* 
 * We always assume a sampling rate of 44100 Hz (T = 0.000022676 s)
 * 
 * Time format is in MSF (minutes, seconds, frames). MM:SS:FF
 * One frame is 1/75 th of one second and has the length of
 * one sector, 2352 bytes.
 * 
 * For the snoopy ones:
 * 2352 byte * 8 = 18816 bit
 * 18816 bit / (16 bit per sample * 2 channels [stereo]) = 588 samples
 * 588 samples * (1 / 44100 Hz [sampling period duration]) = 0.0133... s
 * 0.0133... s * 75 = 1.0 s
 * */

/* ****************************************************************** */

#define BITS_PER_SAMPLE      16
#define CHANNELS              2  /* no of channels 2 = stereo*/
#define EFFECTIVE_BITS       ( BITS_PER_SAMPLE * CHANNELS )
#define EFFECTIVE_BYTES      ( EFFECTIVE_BITS / 8 )

#define SAMPLING_RATE     44100  /* sampling rate in Hz */

#define WAV_HEADER_LEN       44  /* WAV header length in bytes */
#define WAV_EXTENSION    ".wav"

#define FRAMES_PER_SEC       75
#define SECTOR_LEN         2352  /* payload data bytes of one sector */

/* 
 * when reading raw data from a cd da 
 * additional ecc, control and
 * header bytes are stored.
 * e. g. when reading a cd da with
 * cdrdao ... --read-raw ...
 * see corresponding manpages
 * for further information.
 */
#define SECTOR_RAW_LEN     3234 

/* several modes on the disc ... */
#define MODE1_2352 "MODE1/2352"
#define MODE2_2352 "MODE2/2352" /* subcategorize in raw, 
                                   psxtruncate and normal 
                                   when implementing this mode.*/
#define MODE2_2336 "MODE2/2336"
#define MODE_AUDIO "AUDIO"

/* 
 * Block size for processing files located on a filesystem.
 * BLOCK_SIZE is read or written at once.
 * Must be a multiple of one sector!
 * Small block sizes cause a tremendous slowdown.
 */
#define BLOCK_SIZE ( SECTOR_LEN * 14266L ) /* ~ 32 MiB */

/* Multithreading defines */
#define MAX_THREADS 64

/* ****************************************************************** */


typedef struct
{
  
  uint8_t  number;
  
  uint32_t startframe;
  uint32_t endframe;
  
  uint32_t startbyte;
  uint32_t endbyte;
  uint32_t size_byte;
  
  uint32_t offset;    /* byte start offset for some modes */
  uint32_t subsize;   /* in some modes a sector has another
                         size than the SECTOR_LEN */

  uint8_t  is_audio;
  char     mode[ 16 ];

} track_t;


typedef struct
{
  
  char    title[ 16 ];
  char    no[ 8 ];
  char    mode[ 16 ];
  char*   index_str[ 64 ];
  uint8_t index_cnt;

} cueentry_t;


typedef struct
{

  track_t** tracks;
  uint8_t   tracks_len;
  uint8_t   cur_top;

} track_pool_t;

/* ****************************************************************** */

/* "public" function prototypes */

/* ****************************************************************** */
#endif /* WAVER_H_ */

