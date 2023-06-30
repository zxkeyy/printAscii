# printAscii
![Ascii Art Piaktchu](https://i.imgur.com/y3mM7FM.png)

## How to use:
#### Build the c program :
```console
$ gcc printAscii.c -o printAscii -Wall -lm
```
#### Run :
```console
$ ./printAscii [options] [image] AsciiBi2.txt
```
- The order of the arguments must be as specified 
### Available options:
- `-i` : inverses the resulting Ascii image colors.
- `-m 0..2` : changes the mode of calculating which Ascii character is printed. 
- `-a 0..255` : converts pixels with 0 alpha to the value specified.
- `-t 0..255` : changes the threshold for converting pixels to black or white.
- `-w [output width]` : resizes the output to the given width in Ascii characters.
- `-o [output file]` : prints the Ascii art in a file instead of the terminal.

# printVideoAscii
![](https://github.com/zxkeyy/printAscii/blob/master/local/spiningEarthAscii.gif)

## How to use:
- Build with:
```
gcc printVideoAscii.c -o printVideoAscii -Wall
gcc videoAscii.c -o videoAscii -Wall
```
- Turn the video you want to play into a sequence of png pictures named 1.png, 2.png...
- Example with ffmpeg:
```
sudo apt install ffmpeg
ffmpeg -i [video] %d.png
```
- Run the follwing commands to generate a txt file with the video:
```
./videoAscii [output text file] [address of folder where images are] [number of images(frames)]
```
- Run:
```
./playVideoAscii [output text file] [framerate]
```
## Image loading library :
- https://github.com/nothings/stb
