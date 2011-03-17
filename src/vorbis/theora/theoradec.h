/********************************************************************
 *                                                                  *
 * THIS FILE IS PART OF THE OggTheora SOFTWARE CODEC SOURCE CODE.   *
 * USE, DISTRIBUTION AND REPRODUCTION OF THIS LIBRARY SOURCE IS     *
 * GOVERNED BY A BSD-STYLE SOURCE LICENSE INCLUDED WITH THIS SOURCE *
 * IN 'COPYING'. PLEASE READ THESE TERMS BEFORE DISTRIBUTING.       *
 *                                                                  *
 * THE Theora SOURCE CODE IS COPYRIGHT (C) 2002-2003                *
 * by the Xiph.Org Foundation http://www.xiph.org/                  *
 *                                                                  *
 ********************************************************************

  function:
  last mod: $Id: theoradec.h 26239 2007-05-10 01:53:20Z gmaynard $

 ********************************************************************/

/**\file
 * The <tt>libtheora</tt> C decoding API.*/

#if !defined(_O_THEORA_THEORADEC_H_)
# define _O_THEORA_THEORADEC_H_ (1)
# include <ogg/ogg.h>
# include "codec.h"

#if defined(__cplusplus)
extern "C" {
#endif



/**\name theora_decode_ctl() codes
 * \anchor decctlcodes
 * These are the available request codes for theora_decode_ctl().
 * By convention, these are odd, to distinguish them from the
 *  \ref encctlcodes "encoder control codes".
 * Keep any experimental or vendor-specific values above \c 0x8000.*/
/*@{*/
/**Gets the maximum post-processing level.
 *
 * \param[out] _buf int: The maximum post-processing level.
 * \retval OC_FAULT  \a _dec_ctx or \a _buf is <tt>NULL</tt>.
 * \retval OC_EINVAL \a _buf_sz is not <tt>sizeof(int)</tt>.
 * \retval OC_IMPL   Not supported by this implementation.*/
#define OC_DECCTL_GET_PPLEVEL_MAX (1)
/**Sets the post-processing level.
 * By default, post-processing is disabled.
 *
 * \param[in] _buf int: The new post-processing level.
 *                      0 to disable; larger values use more CPU.
 * \retval OC_FAULT  \a _dec_ctx or \a _buf is <tt>NULL</tt>.
 * \retval OC_EINVAL \a _buf_sz is not <tt>sizeof(int)</tt>, or the
 *                    post-processing level is out of bounds.
 *                   The maximum post-processing level may be
 *                    implementation-specific, and can be obtained via
 *                    #OC_DECCTL_GET_PPLEVEL_MAX.
 * \retval OC_IMPL   Not supported by this implementation.*/
#define OC_DECCTL_SET_PPLEVEL (3)
/**Sets the granule position.
 * Call this after a seek, before decoding the first frame, to ensure that the
 *  proper granule position is returned for all subsequent frames.
 *
 * \param[in] _buf <tt>ogg_int64_t</tt>: The granule position of the next
 *                  frame.
 * \retval OC_FAULT  \a _dec_ctx or \a _buf is <tt>NULL</tt>.
 * \retval OC_EINVAL \a _buf_sz is not <tt>sizeof(ogg_int64_t)</tt>, or the
 *                    granule position is negative.*/
#define OC_DECCTL_SET_GRANPOS (5)
/**Sets the striped decode callback function.
 * If set, this function will be called as each piece of a frame is fully
 *  decoded in theora_decode_packetin().
 * You can pass in a #theora_stripe_callback with
 *  theora_stripe_callback#stripe_decoded set to <tt>NULL</tt> to disable the
 *  callbacks at any point.
 *
 * \param[in]  _buf #theora_stripe_callback: The callback parameters.
 * \retval OC_FAULT  \a _dec_ctx or \a _buf is <tt>NULL</tt>.
 * \retval OC_EINVAL \a _buf_sz is not
 *                    <tt>sizeof(theora_stripe_callback)</tt>.*/
#define OC_DECCTL_SET_STRIPE_CB (7)
/*@}*/



/**A callback function for striped decode.
 * This is a function pointer to an application-provided function that will be
 *  called each time a section of the image is fully decoded in
 *  theora_decode_packetin().
 * This allows the application to process the section immediately, while it is
 *  still in cache.
 * Note that the frame is decoded bottom to top, so \a _yfrag0 will steadily
 *  decrease with each call until it reaches 0, at which point the full frame
 *  is decoded.
 * The number of fragment rows made available in each call depends on the pixel
 *  format and the number of post-processing filters enabled, and may not even
 *  be constant for the entire frame.
 * If a non-<tt>NULL</tt> \a _granpos pointer is passed to
 *  theora_decode_packetin(), the granule position for the frame will be stored
 *  in it before the first callback is made.
 * If an entire frame is dropped (a 0-byte packet), then no callbacks will be
 *  made at all for that frame.
 * \param _ctx       An application-provided context pointer.
 * \param _buf       The image buffer for the decoded frame.
 * \param _yfrag0    The Y coordinate of the first row of 8x8 fragments
 *                    decoded.
 *                   Multiply this by 8 to obtain the pixel row number in the
 *                    luma plane.
 *                   If the chroma planes are subsampled in the Y direction,
 *                    this will always be divisible by two.
 * \param _yfrag_end The Y coordinate of the first row of 8x8 fragments past
 *                    the newly decoded section.
 *                   If the chroma planes are subsampled in the Y direction,
 *                    this will always be divisible by two.
 *                   I.e., this section contains fragment rows
 *                    <tt>\a _yfrag0 ...\a _yfrag_end -1</tt>.*/
typedef void (*theora_stripe_decoded_func)(void *_ctx,theora_ycbcr_buffer _buf,
 int _yfrag0,int _yfrag_end);

/**The striped decode callback data to pass to #OC_DECCTL_SET_STRIPE_CB.*/
typedef struct{
  /**An application-provided context pointer.
   * This will be passed back verbatim to the application.*/
  void                       *ctx;
  /**The callback function pointer.*/
  theora_stripe_decoded_func  stripe_decoded;
}theora_stripe_callback;



/**\name Decoder state
   The following data structures are opaque, and their contents are not
    publicly defined by this API.
   Referring to their internals directly is unsupported, and may break without
    warning.*/
/*@{*/
/**The decoder context.*/
typedef struct theora_dec_ctx    theora_dec_ctx;
/**Setup information.
   This contains auxiliary information (Huffman tables and quantization
    parameters) decoded from the setup header by theora_decode_headerin() to be
    passed to theora_decode_alloc().
   It can be re-used to initialize any number of decoders, and can be freed
    via theora_setup_free() at any time.*/
typedef struct theora_setup_info theora_setup_info;
/*@}*/



/**\defgroup decfuncs Functions for Decoding*/
/*@{*/
/**\name Functions for decoding
 * You must link to <tt>libtheorabase</tt> and <tt>libtheoradec</tt> if you use
 *  any of the functions in this section.
 *
 * The functions are listed in the order they are used in a typical decode.
 * The basic steps are:
 * - Parse the header packets by repeatedly calling theora_decode_headerin().
 * - Allocate a #theora_dec_ctx handle with theora_decode_alloc().
 * - Call theora_setup_free() to free any memory used for codec setup
 *    information.
 * - Perform any additional decoder configuration with theora_decode_ctl().
 * - For each video data packet:
 *   - Submit the packet to the decoder via theora_decode_packetin().
 *   - Retrieve the uncompressed video data via theora_decode_ycbcr_out().
 * - Call theora_decode_free() to release all decoder memory.*/
/*@{*/
/**Decodes the header packets of a Theora stream.
 * This should be called on the initial packets of the stream, in succession,
 *  until it returns <tt>0</tt>, indicating that all headers have been
 *  processed, or an error is encountered.
 * At least three header packets are required, and additional optional header
 *  packets may follow.
 * This can be used on the first packet of any logical stream to determine if
 *  that stream is a Theora stream.
 * \param _info  A #theora_info structure to fill in.
 *               This must have been previously initialized with
 *                theora_info_init().
 *               The application may immediately begin using the contents of
 *                this structure after the first header is decoded, though it
 *                must continue to be passed in on all subsequent calls.
 * \param _tc    A #theora_comment structure to fill in.
 *               The application may immediately begin using the contents of
 *                this structure after the second header is decoded, though it
 *                must continue to be passed in on all subsequent calls.
 * \param _setup Returns a pointer to additional, private setup information
 *                needed by the decoder.
 *               The contents of this pointer must be initialized to
 *                <tt>NULL</tt> on the first call, and the returned value must
 *                continue to be passed in on all subsequent calls.
 * \param _op    An <tt>ogg_packet</tt> structure which contains one of the
 *                initial packets of an Ogg logical stream.
 * \return A positive value indicates that a Theora header was successfully
 *          processed.
 * \retval 0            The first video data packet was encountered after all
 *                       required header packets were parsed.
 *                      The packet just passed in on this call should be saved
 *                       and fed to theora_decode_packetin() to begin decoding
 *                       video data.
 * \retval OC_FAULT     One of \a _info, \a _tc, or \a _setup was
 *                       <tt>NULL</tt>.
 * \retval OC_BADHEADER \a _op was <tt>NULL</tt>, the packet was not the next
 *                       header packet in the expected sequence, or the format
 *                       of the header data was invalid.
 * \retval OC_VERSION   The packet data was a Theora info header, but for a
 *                       bitstream version not decodable with this version of
 *                       <tt>libtheora</tt>.
 * \retval OC_NOTFORMAT The packet was not a Theora header.
 */
extern int theora_decode_headerin(theora_info *_info,theora_comment *_tc,
 theora_setup_info **_setup,ogg_packet *_op);
/**Allocates a decoder instance.
 * \param _info  A #theora_info struct filled via theora_decode_headerin().
 * \param _setup A #theora_setup_info handle returned via
 *                theora_decode_headerin().
 * \return The initialized #theora_dec_ctx handle.
 * \retval NULL If the decoding parameters were invalid.*/
extern theora_dec_ctx *theora_decode_alloc(const theora_info *_info,
 const theora_setup_info *_setup);
/**Releases all storage used for the decoder setup information.
 * This should be called after you no longer want to create any decoders for
 *  a stream whose headers you have parsed with theora_decode_headerin().
 * \param _setup The setup information to free.
 *               This can safely be <tt>NULL</tt>.*/
extern void theora_setup_free(theora_setup_info *_setup);
/**Encoder control function.
 * This is used to provide advanced control the decoding process.
 * \param _dec    A #theora_dec_ctx handle.
 * \param _req    The control code to process.
 *                See \ref decctlcodes "the list of available control codes"
 *                 for details.
 * \param _buf    The parameters for this control code.
 * \param _buf_sz The size of the parameter buffer.*/
extern int theora_decode_ctl(theora_dec_ctx *_dec,int _req,void *_buf,
 size_t _buf_sz);
/**Submits a packet containing encoded video data to the decoder.
 * \param _dec     A #theora_dec_ctx handle.
 * \param _op      An <tt>ogg_packet</tt> containing encoded video data.
 * \param _granpos Returns the granule position of the decoded packet.
 *                 If non-<tt>NULL</tt>, the granule position for this specific
 *                  packet is stored in this location.
 *                 This is computed incrementally from previously decoded
 *                  packets.
 *                 After a seek, the correct granule position must be set via
 *                  #OC_DECCTL_SET_GRANPOS for this to work properly.
 * \retval 0            Success.
 * \retval OC_FAULT     \a _dec or _op was <tt>NULL</tt>.
 * \retval OC_BADPACKET \a _op does not contain encoded video data.
 * \retval OC_IMPL      The video data uses bitstream features which this
 *                       library does not support.*/
extern int theora_decode_packetin(theora_dec_ctx *_dec,const ogg_packet *_op,
 ogg_int64_t *_granpos);
/**Outputs the next available frame of decoded Y'CbCr data.
 * If a striped decode callback has been set with #OC_DECCTL_SET_STRIPE_CB,
 *  then the application does not need to call this function.
 * \param _dec   A #theora_dec_ctx handle.
 * \param _ycbcr A video buffer structure to fill in.
 *               <tt>libtheora</tt> will fill in all the members of this
 *                structure, including the pointers to the uncompressed video
 *                data.
 *               The memory for this video data is owned by
 *                <tt>libtheora</tt>.
 *               It may be freed or overwritten without notification when
 *                subsequent frames are decoded.
 * \retval 0 Success
 */
extern int theora_decode_ycbcr_out(theora_dec_ctx *_dec,
 theora_ycbcr_buffer _ycbcr);
/**Frees an allocated decoder instance.
 * \param _dec A #theora_dec_ctx handle.*/
extern void theora_decode_free(theora_dec_ctx *_dec);
/*@}*/
/*@}*/



#if defined(__cplusplus)
}
#endif

#endif
