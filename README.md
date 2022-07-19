# icopack

A command line tool to create multi-frame ICO files from PNG source images.
This tool simply creates a valid ICO header and "glues" together the PNG files provided.
Any other types of images will be rejected. `icopack` is the most efficient way to create Favicons.

## Usage

Compile using CMake. Run with `icnspack output.icns input.iconset`.
Example: `icnspack assets/cog.ico assets/ico.iconset`

## Userfull links

> ### 1. [Icon Editor](https://redketchup.io/icon-editor)
> View and edit Windows icons (ICO files) directly from the browser. Create icons in 8-bit, 24-bit, or 32-bit color depth. Extract all the layers of an ICO file as PNG images.

## License

This software is distributed under the GNU General Public License, Version 3. 
See the LICENSE file for more information.

