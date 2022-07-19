// Copyright (c) 2021 Optidash GmbH
// 
// icopack - pack multiple PNG images into an ICO file
//
// Licensed under the GNU General Public License, Version 3 (the "License");
// you may not use this file except in compliance with the License.
//
// You may obtain a copy of the License at
//    https://www.gnu.org/licenses/gpl-3.0.en.html
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "endian.h"
#include "message.h"
#include "file.h"
#include "str.h"

struct format
{
    const char *filename;    
    uint8_t width, height, 
    colorCount, colorPlanes, 
    bitsPerPixel;    
    uint32_t size;
    uint8_t *data;
};

static struct format formats[] = {
    {"icon_16x16.png", 0, 0, 0, 0, 0, 0, ""},
    {"icon_32x32.png", 0, 0, 0, 0, 0, 0, ""},
    {"icon_48x48.png", 0, 0, 0, 0, 0, 0, ""},
    {"icon_64x64.png", 0, 0, 0, 0, 0, 0, ""},
    {"icon_128x128.png", 0, 0, 0, 0, 0, 0, ""}, 
    {"icon_256x256.png", 0, 0, 0, 0, 0, 0, ""}  
};

int main(int argc, const char **argv) {
    if (argc != 3) {
        printf("-------------------------------------------\n");
        printf("Ico Packer\n");
        printf("Combines icon.iconset images into a single ICO file\n");
        printf("Iconset can contain images of sizes <= 256 pixels\n");
        printf("Listing:\n");
        printf("https://docs.microsoft.com/en-us/windows/win32/uxguide/vis-icons#size-requirements\n\n");
        printf("Usage: icnspack <output_ico> <input_incoset>\n\n");
        printf("Example: \n");
        printf("icopack output.ico input.iconset\n");
        printf("-------------------------------------------\n");
        return 1;
    }

    sendMessage("info", "Creating ico: ", str_prbrk (argv[1], "/\\", false));

    for (size_t i = 0; i < sizeof(formats) / sizeof(formats[0]); ++i) {
        char path[1024];
        snprintf(path, sizeof(path), "%s/%s", argv[2], formats[i].filename);

        FILE* fd = fopen(path, "rb");
        if (fd == NULL) {
            sendMessage("warn", "Missing: ", path);
            continue;
        }

        sendMessage("info", "Collecting image info: ", formats[i].filename);

        fseek(fd, 0L, SEEK_END);
        formats[i].size = ftell(fd);
        fseek(fd, 0, SEEK_SET);

        formats[i].data = malloc(formats[i].size);
        if (formats[i].data == NULL) {
            sendMessage("warn", "Memory allocation error. Skipping file: ", path);
            memset(formats[i].data, 0, formats[i].size);

            continue;
        }

        fread(formats[i].data, 1, formats[i].size, fd);

        unsigned char* data = formats[i].data;

        if (data[0] != 0x89 || data[1] != 0x50 || data[2] != 0x4e || data[3] != 0x47) {
            sendMessage("warn", "PNG file signature not present. Skipping file: ", path);
            memset(formats[i].data, 0, formats[i].size);

            continue;
        }

        // the image size and bit depth
        int width = (data[16] << 24) | (data[17] << 16) | (data[18] << 8) | data[19];
        int height = (data[20] << 24) | (data[21] << 16) | (data[22] << 8) | data[23];
        int bits = data[24]; // bits per pixel
        int pixel = data[25]; // pixel type
        switch (pixel) {
            case 2: // RGB triple
                bits *= 3;
                break;
            case 4: // grayscale + alpha channel
                bits *= 2;
                break;
            case 6: // RGB + alpha
                bits *= 4;
                break;
            default:
                break;
        }

        if (width > 256 || height > 256) {
            sendMessage("warn", "Image files cannot be larger than 256x256. Skipping file: ", path);
            memset(formats[i].data, 0, formats[i].size);

            continue; // bad filename passed
        }

        // Fill in ICONDIRENTRY for this image
        formats[i].width = width;
        formats[i].height = height;

        if (bits < 8)
            formats[i].colorCount = (1 << bits); // number of colors
        else if (bits == 8)
            formats[i].colorCount = 255;
        else
            formats[i].colorCount = 0; // non-palette image

        formats[i].colorPlanes = 1; // color planes
        formats[i].bitsPerPixel = bits; //Bits per pixel

        fclose(fd);
    }

    FILE* fd = fopen(argv[1], "w+b");
    if (fd == NULL) {
        sendMessage("warn", "Failed to open output file: ", argv[1]);
        return 1;
    }    

    // Write header
    fwrite_uint8(0, fd);     // ICONDIR structure starts with 0. Because reserved.
    fwrite_uint8(0, fd);     // Second-byte int
    fwrite_uint8(1, fd);     // ICON file (2 = cursor file)
    fwrite_uint8(0, fd);     // Second-byte int

    uint32_t size = 0;
    for (size_t i = 0; i < sizeof(formats) / sizeof(formats[0]); ++i) {
        if (formats[i].data) {
            size += 1;
        }
    }
    
    fwrite_uint8(size, fd);  // NUMBER of images in the file. 
    fwrite_uint8(0, fd);     // Second-byte int

    // Write images directory
    uint32_t offset = 6 + (size * 16); // starting offset of first file data
    for (size_t i = 0; i < sizeof(formats) / sizeof(formats[0]); ++i) {
        if (formats[i].data) {
            fwrite_uint8(formats[i].width, fd);            // Width
            fwrite_uint8(formats[i].height, fd);           // Height
            fwrite_uint8(formats[i].colorCount, fd);       // Color count
            fwrite_uint8(0, fd);                           // Reserved

            fwrite_uint8(formats[i].colorPlanes, fd);      // Color planes
            fwrite_uint8(0, fd);                           // Second-byte int

            fwrite_uint8(formats[i].bitsPerPixel, fd);     // Bits per pixel
            fwrite_uint8(0, fd);                           // Second-byte int

            fwrite_uint32(formats[i].size, fd);             // Size of the bitmap data in bytes
            fwrite_uint32(offset, fd);                      // Offset in the file

            offset += formats[i].size;
        }
    }

    // Write images
    for (size_t i = 0; i < sizeof(formats) / sizeof(formats[0]); ++i) {
        if (formats[i].data) {
            sendMessage("info", "Writing image data: ", formats[i].filename);

            fwrite(formats[i].data, formats[i].size, 1, fd);    //The actual data of image
        }
    }

    sendMessage("info", "Ico created at: ", get_realpath(argv[1]));

    fclose(fd);
}
