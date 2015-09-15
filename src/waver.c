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
# ===================
# File:    waver.c
# Author:  leibupro
#
# Date:    09/2015
#
#==========================================
*/

#include "waver.h"
#include "mtimer.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <sys/stat.h>
#include <string.h>


/* "private" defines */
#define PATH_LEN  1024
#define NAME_LEN   256

#define LINE_LEN  1024
#define MAX_LINES 1024

/* ****************************************************************** */

/* "private" function prototypes */
uint32_t time_to_frames( char* timestr );
void swapb( char* container, uint32_t container_len );
void parse_arguments( int argc, char* argv[] );
int file_exists( const char* file );
void check_opt_str_len( char* optarg, uint16_t len );
void print_usage( void );

track_t** create_track_metadata( int bin_fd, uint8_t* track_cnt );
void release_track_metadata( track_t** tracks, uint8_t tracks_len );
void process_wav_header( int out_fd, track_t* track );
void process_wav_payload( int in_fd, int out_fd, track_t* track );
void write_tracks( track_t** tracks, uint8_t track_cnt, int bin_fd );
int64_t try_strtol( char* str );
void tokenize( char* str, const char* del, char** tokens, uint32_t exp_tokens );

/* ****************************************************************** */

/* globals */
uint8_t swap_bytes = 0;
uint8_t verbose = 0;
char cuefile[ PATH_LEN ]   = { '\0' };
char binfile[ PATH_LEN ]   = { '\0' };
char base_name[ NAME_LEN ] = { '\0' };

/* ****************************************************************** */

void print_usage( void )
{
  fprintf( stdout, "\nUsage: \n"
                   " waver -b binfile -c cuefile -n basename "
                   "[-s] [-v]\n\n"
                   "=============================================\n"
                   " Example: waver -b foo.bin -c foo.cue -n bar\n" 
                   "=============================================\n\n"
                   "   -v   Verbose output\n" 
                   "   -s   Swap bytes in audio tracks\n"
                   "        (swaps every pair of bytes\n"
                   "        in the binary audio stream)\n\n" );
}


int file_exists( const char* file ) 
{
  struct stat buf;
  return ( stat( file, &buf ) == 0 );
}


void check_opt_str_len( char* optarg, uint16_t len )
{
  if( ( strlen( optarg ) + 1 ) > len )
  {
    fprintf( stderr, "argument string too long, max. %d "
                     "characters allowed. exiting ...\n", len );
    print_usage();
    exit( EXIT_FAILURE );
  }
}


void parse_arguments( int argc, char* argv[] )
{
  int option = 0;
  
  uint8_t binflag = 0;
  uint8_t cueflag = 0;
  uint8_t nameflag = 0;
  
  while( ( option = getopt( argc, argv, "b:c:n:sv" ) ) != -1 )
  {
    switch( option )
    {
      case 'b':
      {
        check_opt_str_len( optarg, PATH_LEN );
        if( !file_exists( optarg ) )
        {
          fprintf( stderr, "binfile does not exist, exiting ...\n" );
          print_usage();
          exit( EXIT_FAILURE );
        }
        strncpy( binfile, optarg, PATH_LEN );
        binflag = 1;
        break;
      }
      case 'c':
      {
        check_opt_str_len( optarg, PATH_LEN );
        if( !file_exists( optarg ) )
        {
          fprintf( stderr, "cuefile does not exist, exiting ...\n" );
          print_usage();
          exit( EXIT_FAILURE );
        }
        strncpy( cuefile, optarg, PATH_LEN );
        cueflag = 1;
        break;
      }
      case 'n':
      {
        check_opt_str_len( optarg, ( NAME_LEN - 4 ) );
        strncpy( base_name, optarg, ( NAME_LEN - 4 ) );
        nameflag = 1;
        break;
      }
      case 's':
      {
        swap_bytes = 1;
        break;
      }
      case 'v':
      {
        verbose = 1;
        break;
      }
      default:
      {
        fprintf( stderr, "invalid or missing arguments, exiting ...\n" );
        print_usage();
        exit( EXIT_FAILURE );
        break;
      }
    }
  }
  
  if( binflag == 0 )
  {
    fprintf( stderr, "missing binfile, exiting ...\n" );
    print_usage();
    exit( EXIT_FAILURE );
  }
  if( cueflag == 0 )
  {
    fprintf( stderr, "missing cuefile, exiting ...\n" );
    print_usage();
    exit( EXIT_FAILURE );
  }
  if( nameflag == 0 )
  {
    fprintf( stderr, "missing base name, exiting ...\n" );
    print_usage();
    exit( EXIT_FAILURE );
  }
}


uint32_t time_to_frames( char* timestr )
{
  char timestrcpy[ 32 + 1 ] = { '\0' };
  const char del[ 2 ] = ":";
  char* tokens[ 3 ] = { NULL };
  uint8_t cnt = 0;
  
  int64_t msf[ 3 ];

  uint32_t frames_total;

  
  strncpy( timestrcpy, timestr, 32 );

  tokenize( timestrcpy, del, tokens, 3 );

  if( tokens[ 0 ] == NULL || 
      tokens[ 1 ] == NULL || 
      tokens[ 2 ] == NULL )
  {
    fprintf( stderr, "Failed to tokenize time value, exiting...\n" );
    exit( EXIT_FAILURE );
  }

  for( cnt = 0; cnt < 3; cnt++ )
  {
    msf[ cnt ] = try_strtol( tokens[ cnt ] );
  }

  frames_total = ( uint32_t )(   msf[ 0 ] * 60 * FRAMES_PER_SEC
                               + msf[ 1 ] * FRAMES_PER_SEC
                               + msf[ 2 ] );

  return frames_total;
}


void tokenize( char* str, const char* del, char** tokens, uint32_t exp_tokens )
{
  uint32_t cnt = 0;
  
  *( tokens + cnt )  = strtok( str, del );
  cnt++;

  while( tokens != NULL && cnt < exp_tokens )
  {
    *( tokens + cnt ) = strtok( NULL, del );
    cnt++;
  }
}


int64_t try_strtol( char* str )
{
  int64_t val;
  char* endptr;

  errno = 0;
  
  val = strtol( str, &endptr, 10 );

  /* Check for various possible errors */
  if( ( errno == ERANGE && 
        ( val == LONG_MAX || val == LONG_MIN ) )
      || ( errno != 0 && val == 0 ) ) 
  {
    fprintf( stderr, "strtol failed, exiting ...\n" );
    exit( EXIT_FAILURE );
  }

  if( endptr == str )
  {
    fprintf( stderr, "No digits were found, exiting ...\n" );
    exit( EXIT_FAILURE );
  }

  return val;
}


void swapb( char* container, uint32_t container_len )
{
  uint32_t i;
  char tmp_byte;
  if( ( container_len % 2 ) != 0 )
  {
    fprintf( stderr, "can't swap bytes. "
                     "2 must be a divisor of the block size. " 
                     "exiting ...\n" );
    exit( EXIT_FAILURE );
  }
  
  /* swap every pair of bytes in the container */
  for( i = 0; i < container_len; i += 2 )
  {
    tmp_byte = *( container + i );
    *( container + i ) = *( container + i + 1 );
    *( container + i + 1 ) = tmp_byte;
  }
}


track_t** create_track_metadata( int bin_fd, uint8_t* track_cnt )
{
  uint16_t i;
  track_t** tracks = NULL;
  track_t* cur_track = NULL;
  track_t* prev_track = NULL;
  FILE* cue_fs = NULL;
  
  char** lines = NULL;
  uint16_t lines_cnt = 0;
  
  char* cur_line = NULL;

  char* newline_pos = NULL;
  char* search_pos = NULL;
  
  cueentry_t cue_entries[ 128 ];

  const char del[ 2 ] = " ";
  char* tokens[ 3 ] = { NULL };
  
  uint8_t index_cnt = 0;

  uint64_t last_bin_byte;

  *track_cnt = 0;

  if( ( cue_fs = fopen( cuefile, "r" ) ) == NULL )
  {
    fprintf( stderr, "Failed to open cuefile, exiting ...\n" );
    exit( EXIT_FAILURE );
  }

  /* ...::: parse the cue file :::... */
  /* :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: */
  
  /* we assume a cue file does not have more lines than MAX_LINES */
  if( 
      ( ( lines = ( char** )malloc( sizeof( char* ) * MAX_LINES ) ) == NULL ) ||
      ( ( *( lines + 0 ) = ( char* )malloc( sizeof( char ) * LINE_LEN ) ) == NULL ) 
    )
  {
    fprintf( stderr, "Memory allocation failure. Exiting ...\n" );
    exit( EXIT_FAILURE );
  }
  lines_cnt++;

  /* first of all read all lines of the cue file and store them in lines */
  i = 0;
  while( 
         ( ( fgets( ( cur_line = *( lines + i ) ), ( LINE_LEN - 1 ), cue_fs ) ) != NULL ) &&
         ( i < ( MAX_LINES - 1 ) )
       )
  {
    /* terminate line with a null byte */
    *( cur_line + ( LINE_LEN - 1 ) ) = '\0';

    /* remove '\n'-character in parsed line */
    if( ( newline_pos = strchr( cur_line, '\n' ) ) != NULL )
    {
      *newline_pos = '\0';
      newline_pos = NULL;
    }


    /* track was found */
    if( ( search_pos = strcasestr( cur_line, "TRACK" ) ) != NULL )
    {
      /* set the index count of previous track. */
      if( *track_cnt > 0 )
      {
        cue_entries[ *track_cnt - 1 ].index_cnt = index_cnt;
        index_cnt = 0;
      }
      
      /* handle new track */
      tokenize( search_pos, del, tokens, 3 );
      strncpy( cue_entries[ *track_cnt ].title, tokens[ 0 ], 16 );
      strncpy( cue_entries[ *track_cnt ].no, tokens[ 1 ], 8 );
      strncpy( cue_entries[ *track_cnt ].mode, tokens[ 2 ], 16 );

      memset( tokens, 0x00, sizeof( char* ) * 3 );
      search_pos = NULL;
      ( *track_cnt )++;
    }

    /* index was found */
    if( ( search_pos = strcasestr( cur_line, "INDEX" ) ) != NULL )
    {
      tokenize( search_pos, del, tokens, 3 );
      cue_entries[ *track_cnt - 1 ].index_str[ index_cnt ] = tokens[ 2 ];
      
      memset( tokens, 0x00, sizeof( char* ) * 3 );
      search_pos = NULL;
      index_cnt++;
      if( index_cnt > ( 64 - 1 ) )
      {
        fprintf( stderr, "Ooops. We ran out of space for index-entries. Exiting ...\n" );
        exit( EXIT_FAILURE );
      }
    }
    
    /* allocate space for next line */
    if( ( *( lines + ( i + 1 ) ) = ( char* )malloc( sizeof( char ) * LINE_LEN ) ) == NULL )
    {
      fprintf( stderr, "Memory allocation failure. Exiting ...\n" );
      exit( EXIT_FAILURE );
    }
    lines_cnt++;

    i++;
  }

  /* set index count of the last track ... */
  cue_entries[ *track_cnt - 1 ].index_cnt = index_cnt;
  index_cnt = 0;

  /* :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: */
  

  if( fclose( cue_fs ) != 0 )
  {
    fprintf( stderr, "Failed to close cuefile, exiting ...\n" );
    exit( EXIT_FAILURE );
  }
  cue_fs = NULL;
  
  /* ...::: set up the tracks metadata :::... */
  /* :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: */
  
  if( ( tracks = ( track_t** )malloc( sizeof( track_t* ) * ( *track_cnt ) ) ) == NULL )
  {
    fprintf( stderr, "memory allocation failure, exiting ...\n" );
    exit( EXIT_FAILURE );
  } 

  for( i = 0; i < *track_cnt; i++ )
  {
    if( ( *( tracks + i ) = ( track_t* )malloc( sizeof( track_t ) ) ) == NULL )
    {
      fprintf( stderr, "memory allocation failure, exiting ...\n" );
      exit( EXIT_FAILURE );
    }
    
    if( i > 0 )
    {
      prev_track = cur_track;
      cur_track = NULL;
    }

    cur_track = *( tracks + i );
    
    cur_track->number = ( i + 1 );
    
    /* 
     * we omit pregaps => last index is the startframe 
     *                 => first index of next track is the endframe 
     *
     * to determine the endframe of the last track we calculate back 
     * from the last byte in the binary data stream.
     */
    if( cue_entries[ i ].index_cnt > 1 )
    {
      cur_track->startframe = 
        time_to_frames( cue_entries[ i ].index_str[ ( cue_entries[ i ].index_cnt - 1 ) ] );
    }
    else
    {
      cur_track->startframe = 
        time_to_frames( cue_entries[ i ].index_str[ 0 ] );
    }
    cur_track->startbyte = cur_track->startframe * SECTOR_LEN;
    
    if( i > 0 )
    {
      prev_track->endframe  = time_to_frames( cue_entries[ i ].index_str[ 0 ] );
      prev_track->endbyte   = prev_track->endframe * SECTOR_LEN;
      prev_track->size_byte = prev_track->endbyte - prev_track->startbyte;
    }

    if( strcasestr( cue_entries[ i ].mode, MODE_AUDIO ) != NULL )
    {
      strncpy( cur_track->mode, cue_entries[ i ].mode, 16 );
      cur_track->is_audio = 1;
    }  
  }

  /* set endframe, endbyte and size_byte for the last track */
  if( ( last_bin_byte = lseek( bin_fd, 0, SEEK_END ) ) == (-1) )
  {
    fprintf( stderr, "Failed to seek last byte, exiting ...\n" );
    exit( EXIT_FAILURE );
  }

  cur_track->endframe  =  ( uint32_t )( last_bin_byte / SECTOR_LEN );
  cur_track->endbyte   = last_bin_byte;
  cur_track->size_byte = cur_track->endbyte - cur_track->startbyte;

  
  /* :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: */


  /* ...::: clean things up :::... */
  /* :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: */
  
  for( i = 0; i < lines_cnt; i++ )
  {
    free( *( lines + i ) );
    *( lines + i ) = NULL;
  }
  free( lines );
  lines = NULL;
  
  /* :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: */
    
  return tracks;
}


void release_track_metadata( track_t** tracks, uint8_t tracks_len )
{
  uint8_t i;

  for( i = 0; i < tracks_len; i++ )
  {
    free( *( tracks + i ) );
    *( tracks + i ) = NULL;
  }

  free( tracks );
  tracks = NULL;
}


void process_wav_header( int out_fd, track_t* track )
{
  uint32_t* uint32_ptr = NULL;
  uint16_t* uint16_ptr = NULL;
  char buf[ WAV_HEADER_LEN ] = { 0 };
  uint32_t overall_file_size = WAV_HEADER_LEN + track->size_byte - 8;
  int errsv;
  int bytes_written;
  
  /* concatenate the 44 bytes of the WAV header */
  /* **************************************************************** */
  
  /* 
   * !!! KEEP IN MIND !!!
   * we live in a little endian world (x86) ...
   * verify header with e. g.: hexdump -C -n 44
   * */
  
  /* 00 - 03   Marks the file as a riff file */
  strncpy( ( buf + 0 ), "RIFF", 4 );     
  
  /* 04 - 07   Size of the overall file minus (-) 8 bytes in, bytes */
  uint32_ptr = ( uint32_t* )&buf[ 4 ];
  *uint32_ptr = overall_file_size;

  /* 08 - 11   File Type Header */
  strncpy( ( buf + 8 ), "WAVE", 4 );

  /* 12 - 15   Format chunk marker. Includes trailing null (blank space) */
  strncpy( ( buf + 12 ), "fmt ", 4 );

  /* 16 - 19   Length of format data as listed above (00-15) */
  uint32_ptr = ( uint32_t* )&buf[ 16 ];
  *uint32_ptr = 16;

  /* 20 - 21   Type of format (1 is PCM) - 2 byte integer */
  uint16_ptr = ( uint16_t* )&buf[ 20 ];
  *uint16_ptr = 1;
  
  /* 22 - 23   Number of Channels - 2 byte integer, 2 = stereo */
  uint16_ptr = ( uint16_t* )&buf[ 22 ];
  *uint16_ptr = 2;

  /* 24 - 27   Sampling rate */
  uint32_ptr = ( uint32_t* )&buf[ 24 ];
  *uint32_ptr = SAMPLING_RATE;

  /* 28 - 31   (Sample Rate * BitsPerSample * Channels) / 8 */
  uint32_ptr = ( uint32_t* )&buf[ 28 ];
  *uint32_ptr = ( SAMPLING_RATE * 
                  BITS_PER_SAMPLE * 
                  CHANNELS ) / 8;
  
  /* 32 - 33   Frame size = <Number of channels> * 
                            ( ( <Bits/Sample (of one channel)> + 7 ) / 8 ) 
               Caution: (Integer division!!) */
  uint16_ptr = ( uint16_t* )&buf[ 32 ];
  *uint16_ptr = ( CHANNELS * 
                  ( uint16_t )( ( BITS_PER_SAMPLE + 7 ) / 8 ) );

  /* 34 - 35   Bits per sample */
  uint16_ptr = ( uint16_t* )&buf[ 34 ];
  *uint16_ptr = BITS_PER_SAMPLE;
  
  /* 36 - 39   "data" chunk header. 
               Marks the beginning of the data section */
  strncpy( ( buf + 36 ), "data", 4 );
  
  /* 40 - 43   Size of the data section (payload size) */
  uint32_ptr = ( uint32_t* )&buf[ 40 ];
  *uint32_ptr = track->size_byte;

  if( ( bytes_written = write( out_fd, buf, WAV_HEADER_LEN ) ) != WAV_HEADER_LEN )
  {
    errsv = errno;
    fprintf( stderr, "Failed to write wav header, " 
             "bytes written: %d\n"
             "errno: %s, exiting ...\n", bytes_written, strerror( errsv ) );
    exit( EXIT_FAILURE );
  }
}


void process_wav_payload( int in_fd, int out_fd, track_t* track )
{
  /* buffer on the heap to prevent stack overflows */
  char* buf = NULL;
  int bytes_read;
  int bytes_written;
  int errsv;
  uint32_t cur_block_size = BLOCK_SIZE;
  uint32_t pieces_count = 0;
  uint32_t overlap_bytes = 0;
  uint32_t i;

  if( ( buf = ( char* )calloc( BLOCK_SIZE, sizeof( char ) ) ) == NULL )
  {
    fprintf( stderr, "Failed to allocate memory for buffer, exiting ...\n" );
    exit( EXIT_FAILURE );
  }
  
  /* **************************************************************** */
  
  pieces_count = ( uint32_t )( track->size_byte / BLOCK_SIZE );
  overlap_bytes = track->size_byte % BLOCK_SIZE;
  if( overlap_bytes > 0 )
  {
    pieces_count++;
  }
  
  /* set file pointer on in file to start position */
  if( lseek( in_fd, track->startbyte, SEEK_SET ) == (-1) )
  {
    fprintf( stderr, "Failed to seek track start byte, exiting ...\n" );
    exit( EXIT_FAILURE );
  }

  /* read block by block and write to output file ... */
  for( i = 0; i < pieces_count; i++ )
  {
    /* case for the last piece */
    if( overlap_bytes > 0 && i == ( pieces_count - 1 ) )
    {
      cur_block_size = overlap_bytes;
    }
    
    if( ( bytes_read = read( in_fd, buf, cur_block_size ) ) != cur_block_size )
    {
      errsv = errno;
      fprintf( stderr, "Failed to read block of data, " 
               "read bytes: %d\n"
               "errno: %s, exiting ...\n", bytes_read, strerror( errsv ) );
      exit( EXIT_FAILURE );
    }
    if( swap_bytes )
    {
      swapb( buf, cur_block_size );
    }
    if( ( bytes_written = write( out_fd, buf, cur_block_size ) ) != cur_block_size )
    {
      errsv = errno;
      fprintf( stderr, "Failed to write block of data, " 
               "bytes written: %d\n"
               "errno: %s, exiting ...\n", bytes_written, strerror( errsv ) );
      exit( EXIT_FAILURE );
    }
  }
  free( buf );
  buf = NULL;
}


void write_tracks( track_t** tracks, uint8_t track_cnt, int bin_fd )
{
  int out_fd = (-1);
  uint8_t i;
  char wav_name[ PATH_LEN ] = { '\0' };
  char track_no[ 3 ] = { '\0' };

  for( i = 0; i < track_cnt; i++ )
  {
    strncat( wav_name, base_name, ( PATH_LEN - 7 ) );
    strncat( wav_name, "_", 1 );
    sprintf( track_no, "%02d", ( i + 1 ) );
    strncat( wav_name, track_no, 2 );
    strncat( wav_name, WAV_EXTENSION, 4 );
    
    out_fd = open( wav_name, 
                   O_WRONLY | O_CREAT | O_TRUNC,
                   S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH );
    
    if( out_fd < 0 )
    {
      fprintf( stderr, "Failed to open output wav file at %d, exiting ...\n", i );
      exit( EXIT_FAILURE );
    }
    
    process_wav_header( out_fd, *( tracks + i ) );
    process_wav_payload( bin_fd, out_fd, *( tracks + i ) );
    
    if( close( out_fd ) != 0 )
    {
      fprintf( stderr, "Failed to close wav file fd at %d, exiting ...\n", i );
      exit( EXIT_FAILURE );
    }
    out_fd = (-1);
    memset( track_no, '\0', 3 );
    memset( wav_name, '\0', PATH_LEN );
  }
}


int main( int argc, char* argv[] )
{
  ttimer_t timer;

  int bin_fd = (-1);
 
  track_t** tracks = NULL;
  uint8_t track_cnt = 0;

  parse_arguments( argc, argv );

  startTTimer( timer );

  bin_fd = open( binfile, O_RDONLY | O_SYNC );

  if( bin_fd < 0 )
  {
    fprintf( stderr, "Failed to open requested files, exiting ...\n" );
    exit( EXIT_FAILURE );
  }
  
  tracks = create_track_metadata( bin_fd, &track_cnt );
  
  write_tracks( tracks, track_cnt, bin_fd );
 
  if( close( bin_fd ) != 0 )
  {
    fprintf( stderr, "Failed to close bin file fd, exiting ...\n" );
    exit( EXIT_FAILURE );
  }

  release_track_metadata( tracks, track_cnt );

  stopTTimer( timer );

  printTTime( timer );

  return EXIT_SUCCESS;
}

