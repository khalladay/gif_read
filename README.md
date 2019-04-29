# gif_read
Dependency free gif parsing in C++

## Why?
I wanted to write a quick app that needed to play some animated gifs, and it felt like overkill to use giflib. So like any bored programmer I decided that it was totally not overkill to write my own gif parser instead. 

## How does it work? 
It's just two files, and the intent is to copy and paste them into whatever project needs them rather than build them as a separate lib. 

gif_read.h provides three different classes for reading gif data - GIF, StreamingGIF, and StreamingCompressedGIF

The GIF is intended for times when you need to access the frames in a gif in random order. Maybe you want to implement seeking through a gif, or just need frame #8, or want to output the frames in reverse order. Whatever. GIF uses a metric crap ton of memory, since it will decompress a gif into uint8 arrays of RGBA color data, and keep all of them in memory (necessary because gif is a streaming format, and the contents of one frame usually depend on the contents of the previous frame, so you can't decompress things on the fly if you're seeking out of order). 

StreamingGIF and StreamingCompressedGIF are both for when you only ever want to see the frames of an animated gif in the normal order. Generally, you'd use one of these if you wanted to implement a gif player, for example. The only difference between them is the tradeoff between CPU usage and Memory. StreamingGIF will decompress the gif into index streams rather than uint8 arrays, and when needed, will convert a frame's index stream into a color array. Memory usage wise, it's about 60% smaller than the GIF class. It keeps an index stream per frame in memory, along with 2 color buffers - one for the first frame, and one for the current frame. 

StreamingCompressedGIF just stores the compressed gif data for each frame. When a new frame is needed, it will decompress this data into a uint8 color array. Just like StreamingGIF, it keeps first frame, and current frame in memory as well. This is a ton smaller in memory, but uses more cpu when ticking to a new frame.

To use any of these classes, first read in the contents of a gif file using your favourite file IO functions, and then instantiate a GIF object, passing the binary contents of that gif file to the ctor: 

    FILE* fp = fopen([file.path cStringUsingEncoding:NSUTF8StringEncoding], "rb");
    fseek(fp, 0, SEEK_END);
    size_t len = ftell(fp);
    
    uint8_t* gifData = (uint8_t*)malloc(len);
    rewind(fp);
    fread(gifData, len, 1, fp);
    
    gif_read::GIF myGif(gifData);
    gif_read::StreamingGIF myStreamingGIF(gifData);
    gif_read::StreamingCompressedGIF myCompressedGif(gifData);

    free(gifData);
    fclose(fp);
    
Notice that after you construct any of these objects, you can free the gifData pointer used to construct it. All three of the classes provided will memcpy the needed data out of the pointer and don't require the original file contents once construction is complete. 

Using the GIF class is straightforward, you just request what frame you want, ie: 

`
[_renderer updateGifTexture:_gif->getFrame(7)];
`

The Streaming types use a tick() function for you to advance the time of the gif. This returns true if a new frame is needed to display the contents of the gif at the current time. 

`
if (_gif->tick(0.016))
{
    [_renderer updateGifTexture:_gif->getCurrentFrame()];
}
`

## Caveats
Currently this has only been tested on OS X. Some things, like the GT_PACKED macro, and my judicious use of memset will need some adjustments if you're using the code on Windows, or on a compiler other than Clang. 

The code doesn't support interlaced gifs, or gifs with sorted color tables. It made the code simpler, and in practice, none of the gifs I wanted to decompress used these features. If you're running in debug, the code will assert if it encounters either of these flags. In release it'll just try it's best and probably crash or display weirdly. 
