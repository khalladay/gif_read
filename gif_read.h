//
//  Gif.h
//  gif_read
//
//  Created by Kyle Halladay on 3/31/19.
//  Copyright © 2019 Kyle Halladay. All rights reserved.
//

//currently only tested / used on OSX 10.14.3

#pragma once
#include <stdint.h> //if you hate stdint, replace this and the typedefs below with your own integer types

//does not support interlaced or sorted gifs. Debug mode will assert if it encounters a gif like this,
//all asserts are disabled in release mode, so it's recommended that you run at least once in debug to
//verify your gifs are compatible
namespace gif_read
{
    typedef uint16_t uint16;
    typedef uint32_t uint32;
    typedef uint8_t  uint8;
    typedef int32_t int32;
    
    //make sure any replacement types are still the right size
    static_assert(sizeof(uint16) == 2, "uint16 type is an incorrect size");
    static_assert(sizeof(uint32) == 4, "uint32 type is an incorrect size");
    static_assert(sizeof(uint8) == 1, "uint8 type is an incorrect size");
    static_assert(sizeof(int32) == 4, "int32 type is an incorrect size");
    
    //memory heavy GIF class that provides access to any frame of a GIF in arbitrary order
    //keeps a uint8 rgb array of every frame in memory all the time, giving the fastest access to
    //data at runtime, at a large memory cost.
    class GIF
    {
    public:
        //gifFileData is the binary contents of a .gif file. Ctor will memcpy
        //out of this data, but doesn't need it after the ctor finishes.
        //dealloc the gifFileData ptr yourself after constructing a GIF
        GIF( const uint8* gifFileData );
        ~GIF();
        
        uint32 getWidth() const;
        uint32 getHeight() const;
        uint32 getNumFrames() const;
        
        //returns an array of unsigned byte RGBA pixel data for a texture with the dimensions
        //defined by the getWidth() and getHeight() function calls. Alpha will always be 255
        const uint8* getFrame(uint32 frameIndex) const;
        const uint8* getFrameAtTime(float time, bool looping = true) const;
        
    private:
        struct GIFImpl* _impl = nullptr;
    };
    
    //Instead of storing index streams, only the compressed gif data is stored, and is decompressed as new frames are needed
    //To support multiple instances of a GIF displaying different frames, StreamingGIFs are accessed through gif-erators
    //(my stupid name for gif iterators), which require an allocation of 1 frame of gif data each. The memory for all
    //giferators is handled by the streamingGIF class they're allocated by, and accessed through a uint32 handle.
    class StreamingGIF
    {
    public:
        //gifFileData is the binary contents of a .gif file. Ctor will memcpy
        //out of this data, but doesn't need it after the ctor finishes.
        //dealloc the gifFileData ptr yourself after construction
        StreamingGIF( const uint8* gifFileData, uint32 maxIterators = 8 );
        ~StreamingGIF();
        StreamingGIF(const StreamingGIF&) = delete;
        StreamingGIF& operator=(const StreamingGIF&) = delete;
        
        uint32 getWidth() const;
        uint32 getHeight() const;
        uint32 getNumFrames() const;
        float getDurationInSeconds() const;
        
        uint32 createIterator();
        bool isIteratorValid(uint32 iterator);
        void destroyIterator(uint32 iterator);
        
        //returns true if time has advanced enough to get a new frame
        bool tickSingleIterator(uint32 interator, float deltaTime);
        void tick(float deltaTime); //ticks all iterators
        
        const uint8* getFirstFrame() const;
        const uint8* getCurrentFrame(uint32 interator) const;
        
    protected:
        struct StreamingGIFImpl* _impl = nullptr;
        
    };
}
