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
    
    //interface for streaming GIF classes
    class IStreamingGIF
    {
    public:
        uint32 getWidth() const;
        uint32 getHeight() const;
        uint32 getNumFrames() const;
        float getDurationInSeconds() const;
        
        //both of these return an array of unsigned byte RGBA pixel data
        //Alpha will always be 255
        const uint8* getCurrentFrame() const;
        const uint8* getFirstFrame() const;
        
        virtual bool tick(float deltaTime) = 0;
        
        IStreamingGIF();
        virtual ~IStreamingGIF();

        IStreamingGIF(const IStreamingGIF&) = delete;
        IStreamingGIF& operator=(const IStreamingGIF&) = delete;
    protected:
        struct StreamingGIFImpl* _impl = nullptr;
    };
    
    //lighter GIF class that provides access only to the first frame of a gif and the current frame
    //being played. Frames can only advance linearly, and advancing the frame requires some CPU to merge
    //the next frame's data with the current frame array. Roughly 60% smaller in memory than the GIF class
    class StreamingGIF : public IStreamingGIF
    {
    public:
        
        //gifFileData is the binary contents of a .gif file. Ctor will memcpy
        //out of this data, but doesn't need it after the ctor finishes.
        //dealloc the gifFileData ptr yourself after construction
        StreamingGIF( const uint8* gifFileData);
        virtual ~StreamingGIF();
        
        //returns true if time has advanced enough to get a new frame
        //will also update currentFrame if necessary. tick() will only ever
        //increment the current frame by 1. If you provide a timestep that
        //requires jumping multiple frames, it will take severall calls to
        //tick() to catch up. This is intentional, to prevent CPU hitches
        //from causing atypical CPU usage when encountering a hitch in a
        //running game
        virtual bool tick(float deltaTime) override final;
    };
    
    //even lighter gif class on memory, at the cost of CPU time every new frame. Instead of storing
    //index streams, only the compressed gif data is stored, and is decompressed as new frames are needed
    //Size savings depend on quality of gif compression, but can be upwards of 3x smaller than regular streaming gif
    class StreamingCompressedGIF : public IStreamingGIF
    {
    public:
        //gifFileData is the binary contents of a .gif file. Ctor will memcpy
        //out of this data, but doesn't need it after the ctor finishes.
        //dealloc the gifFileData ptr yourself after construction
        StreamingCompressedGIF( const uint8* gifFileData );
        virtual ~StreamingCompressedGIF();
        
        //returns true if time has advanced enough to get a new frame
        //will also update currentFrame if necessary. See notes in StreamingGIF
        //for further info.
        bool tick(float deltaTime) override final;
    };
}
