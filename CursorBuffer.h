//
//  CursorBuffer.h
//
//  Created by David O'Neill on 8/17/17.
//  Copyright © 2017 O'Neill. All rights reserved.
//

#ifndef CursorBuffer_h
#define CursorBuffer_h

#include "TPCircularBuffer+AudioBufferList.h"



typedef struct{
    TPCircularBuffer buffer;
    TPCircularBufferABLBlockHeader *cursor;
    AudioStreamBasicDescription asbd;
    TPCircularBufferABLBlockHeader *head;
}CursorBuffer;


/*!
 * Initialises buffer
 *
 * Will initialize internal TPCircularBuffer to hold more than the frames specified using the format asbd.
 *
 * @param cursorBuffer Cursor buffer.
 * @param format Format of audioBufferlists that will be copied into the buffer, must me non-interleaved.
 * @param frames Specifies the minimum frames of audio the buffer will hold.
 */
void CursorBufferInit(CursorBuffer *cursorBuffer, AudioStreamBasicDescription format, int32_t frames);

/*!
 * Frees internal buffers
 * 
 * @param cursorBuffer Cursor buffer.
 */
void CursorBufferCleanup(CursorBuffer *cursorBuffer);

/*!
 * Gets the number of frames that can be written without over-flowing.
 *
 * @param cursorBuffer Cursor buffer.
 */
UInt32 CursorBufferGetAvailableSpace(CursorBuffer *cursorBuffer);

/*!
 * Copies an AudioBufferlist with timestamp onto internal buffer
 *
 * @param cursorBuffer Cursor buffer.
 * @param inBufferList AudioBufferlist to copy.
 * @param inTimestamp Time stamp to be stored along with the buffer.
 * @param frames Number of frames contained in inBufferList
 */
bool CursorBufferCopyAudioBufferList(CursorBuffer *cursorBuffer, const AudioBufferList *inBufferList, const AudioTimeStamp *inTimestamp, UInt32 frames);

/*!
 * Consumes the first AudioBufferlist in the buffer
 *
 * This can potentially invalidate cursor.
 *
 * @param cursorBuffer Cursor buffer.
*/
void CursorBufferConsume(CursorBuffer *cursorBuffer);

/*! 
 * Moves cursor to the buffer containing sample postion specified.
 *
 * If not buffer is found containing sample, then NULL is
 * returned, and internal cursor set to NULL.
 *
 * This can potentially invalidate cursor.
 * 
 * @param cursorBuffer Cursor buffer.
 * @param sampleTime The sample position that the returned buffer should contain.
 * @param timeStamp On output, the timeStamp of the returned AudioBufferlist
 * @return Pointer to an AudioBufferlist in the buffer that contains the sampleTime specified, or NULL if not found.
 */
AudioBufferList *CursorBufferSeek(CursorBuffer *cursorBuffer, Float64 sampleTime, AudioTimeStamp *timeStamp);

/*!
 *  Moves cursor to first AudioBufferlist in buffer.
 *
 * @param cursorBuffer Cursor buffer.
 * @param timeStamp On output, the timeStamp of the returned AudioBufferlist
 * @return AudioBufferlist in the buffer at cursor, or NULL if empty.
 */
AudioBufferList *CursorBufferStart(CursorBuffer *cursorBuffer, AudioTimeStamp *timeStamp);

/*!
 * Gets current AudioBufferlist at cursor.
 *
 * @param cursorBuffer Cursor buffer.
 * @param timeStamp On output, the timeStamp of the returned AudioBufferlist
 * @return AudioBufferlist in the buffer at cursor, or NULL if empty.
 */
AudioBufferList *CursorBufferCurrent(CursorBuffer *cursorBuffer, AudioTimeStamp *timeStamp);


/*!
 * Used to determine if cursor is valid.
 * 
 * @param cursorBuffer Cursor buffer.
 * @return True if cursor is valid, false if not.
 */
bool CursorBufferHasCurrent(CursorBuffer *cursorBuffer);

/*!
 * Used to determine if cursor can advance.
 *
 * @param cursorBuffer Cursor buffer.
 * @return True if cursor is valid and can advance, false if not.
 */
bool CursorBufferCanAdvance(CursorBuffer *cursorBuffer);

/*!
 * Advances cusor to the next AudioBufferlist in the buffer.
 *
 * If cursor is at the end of the buffer, and there is no buffer to
 * advance to, then NULL is returned, and the cursor retains it's position.
 *
 * @param cursorBuffer Cursor buffer.
 * @param timeStamp On output, the timeStamp of the returned AudioBufferlist
 * @return AudioBufferlist in the buffer cursor was advanced to, or NULL if cursor could not advance.
 */
AudioBufferList *CursorBufferAdvance(CursorBuffer *cursorBuffer, AudioTimeStamp *timeStamp);


/*!
 * Determine whether or not timeStamp is a continuation of samples in buffer.
 *
 * Safe to call from producer thread.
 *
 * @param cursorBuffer Cursor buffer.
 * @param inTimestamp Timestamp to chack against last buffer sample.
 * @param tolerance In samples. Allows a not exact match to still be considered contiguos. 
 * @return Is contiguos.
 */
bool CursorBufferCheckContiguos(CursorBuffer *cursorBuffer, const AudioTimeStamp *inTimestamp, Float64 tolerance);





#endif /* CursorBuffer_h */
