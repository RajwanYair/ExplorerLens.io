# Minimal Plugin Example

This is the simplest possible DarkThumbs plugin that demonstrates the basic structure.

## What It Does

Decodes a custom text-based image format called "TXTIMG" where ASCII characters represent pixels:
- Space ` ` = White pixel
- `#` = Black pixel  
- `@` = Very dark gray
- `+` = Medium gray
- `.` = Light gray

## Format Specification

```
TXTIMG <width> <height>
<ASCII art data>
```

### Example File (8x8 box):

```
TXTIMG 8 8
########
#      #
# #### #
# #  # #
# #  # #
# #### #
#      #
########
```

## Building

```bash
mkdir build && cd build
cmake -G "Visual Studio 18 2026" -A x64 ..
cmake --build . --config Release
```

## Testing

Create a test file `test.txtimg`:

```
TXTIMG 16 16
################
#              #
#  ##########  #
#  #        #  #
#  #  ####  #  #
#  #  #  #  #  #
#  #  #  #  #  #
#  #  ####  #  #
#  #        #  #
#  ##########  #
#              #
################
```

Test with:

```bash
plugin-tester.exe minimal_plugin.dll test.txtimg
```

## Code Structure

- `plugin_get_info()` - Returns plugin metadata
- `plugin_init()` - Stores memory allocator
- `plugin_cleanup()` - No-op (nothing to clean)
- `plugin_can_decode()` - Checks ".txtimg" extension or "TXTIMG " magic bytes
- `plugin_decode()` - Parses header, decodes ASCII art, scales to target size
- `plugin_free_result()` - Frees pixel buffer

## Key Concepts Demonstrated

1. **Header Parsing**: Reading format metadata
2. **Format Validation**: Checking magic bytes
3. **Memory Management**: Using host allocator
4. **Image Scaling**: Simple bilinear scaling
5. **Progress Reporting**: Optional progress callbacks
6. **Error Handling**: Proper error codes and messages

## License

MIT License - See LICENSE.txt
