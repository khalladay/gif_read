# gif_read
Dependency free gif parsing in C++

## Why?
I wanted to write a quick app that needed to play some animated gifs, and it felt like overkill to use giflib. So like any bored programmer I decided that it was totally not overkill to write my own gif parser instead. 

## How does it work? 
It's just two files, and the intent is to copy and paste them into whatever project needs them rather than build them as a separate lib. 

gif_read.h provides two different classes for reading gif data - GIF and StreamingGIF

The GIF is intended for times when you need to access the frames in a gif in random order. Maybe you want to implement seeking through a gif, or just need frame #8, or want to output the frames in reverse order. Whatever. GIF uses a metric crap ton of memory, since it will decompress a gif into uint8 arrays of RGBA color data, and keep all of them in memory (necessary because gif is a streaming format, and the contents of one frame usually depend on the contents of the previous frame, so you can't decompress things on the fly if you're seeking out of order). 

StreamingGIF is for when you only ever want to see the frames of an animated gif in the normal order. Generally, you'd use one of these if you wanted to implement a gif player, for example. It works by storing the compressed data for each frame and decompressing it into uint8 arrays when that frame is needed. 


To use any of these classes, first read in the contents of a gif file using your favourite file IO functions, and then instantiate a GIF object, passing the binary contents of that gif file to the ctor: 

    FILE* fp = fopen([file.path cStringUsingEncoding:NSUTF8StringEncoding], "rb");
    fseek(fp, 0, SEEK_END);
    size_t len = ftell(fp);
    
    uint8_t* gifData = (uint8_t*)malloc(len);
    rewind(fp);
    fread(gifData, len, 1, fp);
    
    gif_read::GIF myGif(gifData);
    gif_read::StreamingGIF myStreamingGIF(gifData);

    free(gifData);
    fclose(fp);
    
Notice that after you construct any of these objects, you can free the gifData pointer used to construct it. All three of the classes provided will memcpy the needed data out of the pointer and don't require the original file contents once construction is complete. 

Using the GIF class is straightforward, you just request what frame you want, ie: 

`
[_renderer updateGifTexture:_gif->getFrame(7)];
`

StreamingGIF data is accessed by using an iterator. To create an iterator, call StreamingGIF::createIterator(), which will return a uint32 handle to one. Each iterator can store it's own timestep, and currently displayed frame, to support multiple instances of the same gif at different frames, without duplicating compressed data. You can destroy or tick individual iterators (which have a memory cost of 1 frame of decompressed gif data) by using the following functions: 



    //returns true if time has advanced enough to get a new frame
    bool tickSingleIterator(uint32 interator, float deltaTime);

    bool isIteratorValid(uint32 iterator);

    void destroyIterator(uint32 iterator);


You can also tick all iterators at once using the StreamingGIF::tick() function. 
When a StreamingGIF is destroyed, all iterators are destroyed with it. 

## Caveats
Currently this has only been tested on OS X. Some things, like the GT_PACKED macro, and my judicious use of memset will need some adjustments if you're using the code on Windows, or on a compiler other than Clang. 

The code doesn't support interlaced gifs, or gifs with sorted color tables. It made the code simpler, and in practice, none of the gifs I wanted to decompress used these features. If you're running in debug, the code will assert if it encounters either of these flags. In release it'll just try it's best and probably crash or display weirdly. 

The handling of StreamingGIF iterators isn't as industry-grade as it could be. If you plan on creating and destroying a lot of iterators, you'll want to revisit the creation/destruction logic, and possible add support for a frame pool, rather than having each iterator allocate it's own frame (to support multiple iterators viewing the same decompressed frame without memory duplication).
