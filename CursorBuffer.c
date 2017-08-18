//
//  CursorBuffer.c
//
//  Created by David O'Neill on 8/17/17.
//  Copyright © 2017 O'Neill. All rights reserved.
//

#include "CursorBuffer.h"


static bool headerContainsSample(TPCircularBufferABLBlockHeader *header, Float64 sampleTime, int bytesPerFrame);
static void *zeroTimeStamp(AudioTimeStamp *timeStamp);




void CursorBufferInit(CursorBuffer *cursorBuffer, AudioStreamBasicDescription asbd, int32_t frames) {
    assert(asbd.mFormatFlags & kAudioFormatFlagIsNonInterleaved);

    int bytes = asbd.mBytesPerFrame * asbd.mChannelsPerFrame * frames;
    bytes *= 1.1;  //Added 10% for header size

    TPCircularBufferInit(&cursorBuffer->buffer, bytes);
    cursorBuffer->cursor = NULL;
    cursorBuffer->asbd = asbd;
    cursorBuffer->head = NULL;
}

void CursorBufferCleanup(CursorBuffer *cursorBuffer) {
    TPCircularBufferCleanup(&cursorBuffer->buffer);
    cursorBuffer->cursor = NULL;
    cursorBuffer->head = NULL;

}

UInt32 CursorBufferGetAvailableSpace(CursorBuffer *cursorBuffer){
    return TPCircularBufferGetAvailableSpace(&cursorBuffer->buffer, &cursorBuffer->asbd);
}

bool CursorBufferCheckContiguos(CursorBuffer *cursorBuffer, const AudioTimeStamp *inTimestamp, Float64 tolerance){
    if (!cursorBuffer->head) return true;
    int samplesInBuffer = cursorBuffer->head->bufferList.mBuffers[0].mDataByteSize / cursorBuffer->asbd.mBytesPerFrame;
    int endSample = cursorBuffer->head->timestamp.mSampleTime + samplesInBuffer;
    return endSample - tolerance <= inTimestamp->mSampleTime && inTimestamp->mSampleTime <= endSample + tolerance;
}

void CursorBufferClear(CursorBuffer *cursorBuffer) {
    TPCircularBufferClear(&cursorBuffer->buffer);
    cursorBuffer->cursor = NULL;
    cursorBuffer->head = NULL;

}

bool CursorBufferCopyAudioBufferList(CursorBuffer *cursorBuffer, const AudioBufferList *inBufferList, const AudioTimeStamp *inTimestamp, UInt32 frames){
    int32_t availableBytes = 0;
    TPCircularBufferABLBlockHeader *header = TPCircularBufferHead(&cursorBuffer->buffer, &availableBytes);
    bool success = TPCircularBufferCopyAudioBufferList(&cursorBuffer->buffer, inBufferList, inTimestamp, frames, &cursorBuffer->asbd);
    cursorBuffer->head = header;
    return success;
}

void CursorBufferConsume(CursorBuffer *cursorBuffer) {

    TPCircularBuffer *buffer = &cursorBuffer->buffer;

    int32_t availableBytes;
    void *tail = TPCircularBufferTail(buffer, &availableBytes);
    if ( !tail ){
        cursorBuffer->cursor = NULL;
        return;
    }

    TPCircularBufferABLBlockHeader *block = (TPCircularBufferABLBlockHeader*)tail;

    void *cursor = (void *)cursorBuffer->cursor;
    if (cursor && cursor >= (void *)((char *)block + block->totalLength)) {
        long tailToCursor = cursor - tail - block->totalLength;
        TPCircularBufferConsume(&cursorBuffer->buffer, block->totalLength);
        tail = TPCircularBufferTail(buffer, &availableBytes);
        cursor = tail ? (char *)tail + tailToCursor : NULL;
    } else {
        TPCircularBufferConsume(&cursorBuffer->buffer, block->totalLength);
        cursor = NULL;
    }
    cursorBuffer->cursor = cursor;
}

AudioBufferList *CursorBufferSeek(CursorBuffer *cursorBuffer, Float64 sampleTime, AudioTimeStamp *timeStamp){

    int32_t availableBytes;
    void *tail = TPCircularBufferTail(&cursorBuffer->buffer, &availableBytes);
    if (!tail) return zeroTimeStamp(timeStamp);
    void *end = (char *)tail + availableBytes;

    TPCircularBufferABLBlockHeader *header = (TPCircularBufferABLBlockHeader*)tail;
    while (header && !headerContainsSample(header, sampleTime, cursorBuffer->asbd.mBytesPerFrame)) {
        header = (TPCircularBufferABLBlockHeader *)((char *)header + header->totalLength);
        if ( (void *)header >= end ) {
            cursorBuffer->cursor = NULL;
            return zeroTimeStamp(timeStamp);
        }
    }

    cursorBuffer->cursor = header;

    return CursorBufferCurrent(cursorBuffer, timeStamp);
}

AudioBufferList *CursorBufferStart(CursorBuffer *cursorBuffer, AudioTimeStamp *timeStamp) {
    int32_t availableBytes;
    cursorBuffer->cursor = (TPCircularBufferABLBlockHeader *)TPCircularBufferTail(&cursorBuffer->buffer, &availableBytes);
    return CursorBufferCurrent(cursorBuffer, timeStamp);
}

AudioBufferList *CursorBufferCurrent(CursorBuffer *cursorBuffer, AudioTimeStamp *timeStamp) {
    TPCircularBufferABLBlockHeader *cursor = cursorBuffer->cursor;
    if (!cursor) return zeroTimeStamp(timeStamp);
    if (timeStamp) *timeStamp = cursor->timestamp;
    return &cursorBuffer->cursor->bufferList;
}

AudioBufferList *CursorBufferAdvance(CursorBuffer *cursorBuffer, AudioTimeStamp *timeStamp) {
    if (!CursorBufferCanAdvance(cursorBuffer)) return zeroTimeStamp(timeStamp);
    cursorBuffer->cursor = (void *)((char *)cursorBuffer->cursor + cursorBuffer->cursor->totalLength);
    return CursorBufferCurrent(cursorBuffer, timeStamp);
}

bool CursorBufferHasCurrent(CursorBuffer *cursorBuffer){
    return cursorBuffer->cursor != NULL;
}

bool CursorBufferCanAdvance(CursorBuffer *cursorBuffer){
    if (!cursorBuffer->cursor) return false;

    int32_t availableBytes;
    void *tail = TPCircularBufferTail(&cursorBuffer->buffer, &availableBytes);
    void *end = (char *)tail + availableBytes;
    void *cursor = (void *)((char *)cursorBuffer->cursor + cursorBuffer->cursor->totalLength);

    return cursor < end;
}

static void *zeroTimeStamp(AudioTimeStamp *timeStamp) {
    if (timeStamp) memset(timeStamp, 0, sizeof(AudioTimeStamp));
    return NULL;
}

static bool headerContainsSample(TPCircularBufferABLBlockHeader *header, Float64 sampleTime, int bytesPerFrame) {
    Float64 firstSample = header->timestamp.mSampleTime;
    Float64 lastSample = firstSample + (header->bufferList.mBuffers[0].mDataByteSize / bytesPerFrame) - 1;
    return firstSample <= sampleTime && sampleTime <= lastSample;
}

