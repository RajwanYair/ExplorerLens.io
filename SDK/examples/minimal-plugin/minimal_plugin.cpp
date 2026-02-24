/******************************************************************************
 * Minimal ExplorerLens Plugin Example
 * 
 * This is the simplest possible plugin that demonstrates the basic structure.
 * It decodes a custom text-based image format.
 *****************************************************************************/

#include "../../plugin_api.h"
#include <cstring>
#include <cstdio>
#include <algorithm>

//============================================================================
// Custom Format Specification
//============================================================================
// Format: Simple ASCII art format
// Header: "TXTIMG <width> <height>\n"
// Body: ASCII characters where space=' ', '#'=black, others=gray levels
//
// Example:
// TXTIMG 8 8
// ########
// #      #
// # #### #
// # #  # #
// # #  # #
// # #### #
// #      #
// ########
//============================================================================

// Plugin information
static const char* s_extensions[] = {".txtimg", ".txt", NULL};
static const char* s_mime_types[] = {"text/x-txtimg", NULL};

static const PluginInfo s_plugin_info = {
    .plugin_name = "Text Image Decoder",
    .plugin_version = "1.0.0",
    .plugin_author = "ExplorerLens Team",
    .plugin_description = "Simple ASCII art image format decoder",
    .plugin_license = "MIT",
    .api_version = EXPLORERLENS_PLUGIN_API_VERSION,
    .supported_extensions = s_extensions,
    .mime_types = s_mime_types,
    .capabilities = PLUGIN_CAP_STILL_IMAGE,
    .max_threads = 0,
    .requires_gpu = false,
    .supports_background_loading = true,
};

// Global state
static PluginAllocator s_allocator = {0};

//============================================================================
// Helper Functions
//============================================================================

// Parse header: "TXTIMG <width> <height>\n"
static bool ParseHeader(const char* data, size_t size, uint32_t* width, uint32_t* height) {
    if (size < 10) return false;  // Minimum header size
    
    if (memcmp(data, "TXTIMG ", 7) != 0) return false;
    
    // Parse width and height
    if (sscanf(data + 7, "%u %u", width, height) != 2) return false;
    
    // Validate dimensions
    if (*width == 0 || *height == 0 || *width > 4096 || *height > 4096) {
        return false;
    }
    
    return true;
}

// Convert ASCII character to grayscale value
static uint8_t CharToGray(char c) {
    if (c == ' ') return 255;  // White
    if (c == '#') return 0;    // Black
    if (c == '@') return 32;   // Very dark
    if (c == '+') return 128;  // Medium
    if (c == '.') return 192;  // Light
    return 128;  // Default gray
}

// Decode ASCII art to BGRA32 pixels
static bool DecodeToPixels(const char* data, size_t size, 
                          uint32_t width, uint32_t height,
                          uint8_t* pixels, uint32_t stride) {
    // Find start of pixel data (after first newline)
    const char* body = strchr(data, '\n');
    if (!body) return false;
    body++;  // Skip newline
    
    // Decode each row
    for (uint32_t y = 0; y < height; y++) {
        uint8_t* row = pixels + (y * stride);
        
        for (uint32_t x = 0; x < width; x++) {
            if (body >= data + size) {
                return false;  // Unexpected end of data
            }
            
            // Read character
            char c = *body++;
            
            // Skip newlines
            if (c == '\n' || c == '\r') {
                c = *body++;
                if (c == '\n' || c == '\r') c = *body++;
            }
            
            // Convert to grayscale
            uint8_t gray = CharToGray(c);
            
            // Write BGRA pixel (grayscale, so B=G=R)
            row[x * 4 + 0] = gray;  // B
            row[x * 4 + 1] = gray;  // G
            row[x * 4 + 2] = gray;  // R
            row[x * 4 + 3] = 255;   // A (fully opaque)
        }
    }
    
    return true;
}

// Simple bilinear scaling
static void ScaleImage(const uint8_t* src, uint32_t src_width, uint32_t src_height, uint32_t src_stride,
                      uint8_t* dst, uint32_t dst_width, uint32_t dst_height, uint32_t dst_stride) {
    float x_ratio = (float)src_width / dst_width;
    float y_ratio = (float)src_height / dst_height;
    
    for (uint32_t y = 0; y < dst_height; y++) {
        uint8_t* dst_row = dst + (y * dst_stride);
        float sy = y * y_ratio;
        uint32_t sy0 = (uint32_t)sy;
        uint32_t sy1 = std::min(sy0 + 1, src_height - 1);
        float fy = sy - sy0;
        
        for (uint32_t x = 0; x < dst_width; x++) {
            float sx = x * x_ratio;
            uint32_t sx0 = (uint32_t)sx;
            uint32_t sx1 = std::min(sx0 + 1, src_width - 1);
            float fx = sx - sx0;
            
            // Get 4 neighboring pixels
            const uint8_t* p00 = src + (sy0 * src_stride) + (sx0 * 4);
            const uint8_t* p10 = src + (sy0 * src_stride) + (sx1 * 4);
            const uint8_t* p01 = src + (sy1 * src_stride) + (sx0 * 4);
            const uint8_t* p11 = src + (sy1 * src_stride) + (sx1 * 4);
            
            // Bilinear interpolation for each channel
            for (int c = 0; c < 4; c++) {
                float v0 = p00[c] * (1 - fx) + p10[c] * fx;
                float v1 = p01[c] * (1 - fx) + p11[c] * fx;
                float v = v0 * (1 - fy) + v1 * fy;
                dst_row[x * 4 + c] = (uint8_t)v;
            }
        }
    }
}

//============================================================================
// Plugin API Implementation
//============================================================================

PLUGIN_API const PluginInfo* PLUGIN_CALL plugin_get_info(void) {
    return &s_plugin_info;
}

PLUGIN_API PluginErrorCode PLUGIN_CALL plugin_init(const PluginAllocator* allocator) {
    if (!allocator || !allocator->alloc || !allocator->free) {
        return PLUGIN_ERROR_INVALID_PARAMETER;
    }
    
    s_allocator = *allocator;
    return PLUGIN_SUCCESS;
}

PLUGIN_API void PLUGIN_CALL plugin_cleanup(void) {
    // Nothing to cleanup
}

PLUGIN_API bool PLUGIN_CALL plugin_can_decode(const char* file_path, 
                                                const uint8_t* data, 
                                                size_t data_size) {
    // Check file extension
    if (file_path) {
        const char* ext = strrchr(file_path, '.');
        if (ext && strcmp(ext, ".txtimg") == 0) {
            return true;
        }
    }
    
    // Check magic bytes
    if (data && data_size >= 7) {
        return memcmp(data, "TXTIMG ", 7) == 0;
    }
    
    return false;
}

PLUGIN_API PluginErrorCode PLUGIN_CALL plugin_decode(const DecodeRequest* request,
                                                       DecodeResult* result,
                                                       PluginProgressCallback progress) {
    if (!request || !result) {
        return PLUGIN_ERROR_INVALID_PARAMETER;
    }
    
    // Initialize result
    memset(result, 0, sizeof(DecodeResult));
    result->error_code = PLUGIN_SUCCESS;
    
    // Read file data
    const uint8_t* data = request->data;
    size_t data_size = request->data_size;
    
    FILE* file = nullptr;
    if (!data && request->file_path) {
        file = fopen(request->file_path, "rb");
        if (!file) {
            result->error_code = PLUGIN_ERROR_FILE_NOT_FOUND;
            result->error_message = "Failed to open file";
            return PLUGIN_ERROR_FILE_NOT_FOUND;
        }
        
        // Get file size
        fseek(file, 0, SEEK_END);
        data_size = ftell(file);
        fseek(file, 0, SEEK_SET);
        
        // Read file
        uint8_t* file_data = (uint8_t*)s_allocator.alloc(data_size, s_allocator.user_data);
        if (!file_data) {
            fclose(file);
            result->error_code = PLUGIN_ERROR_OUT_OF_MEMORY;
            result->error_message = "Out of memory";
            return PLUGIN_ERROR_OUT_OF_MEMORY;
        }
        
        fread(file_data, 1, data_size, file);
        fclose(file);
        data = file_data;
    }
    
    if (progress) progress(0.2f, request->user_data);
    
    // Parse header
    uint32_t img_width = 0, img_height = 0;
    if (!ParseHeader((const char*)data, data_size, &img_width, &img_height)) {
        result->error_code = PLUGIN_ERROR_DECODE_ERROR;
        result->error_message = "Invalid TXTIMG header";
        if (request->file_path) s_allocator.free((void*)data, s_allocator.user_data);
        return PLUGIN_ERROR_DECODE_ERROR;
    }
    
    if (progress) progress(0.4f, request->user_data);
    
    // Allocate temporary buffer for full-size image
    uint32_t full_stride = img_width * 4;
    size_t full_size = full_stride * img_height;
    uint8_t* full_pixels = (uint8_t*)s_allocator.alloc(full_size, s_allocator.user_data);
    if (!full_pixels) {
        result->error_code = PLUGIN_ERROR_OUT_OF_MEMORY;
        result->error_message = "Out of memory";
        if (request->file_path) s_allocator.free((void*)data, s_allocator.user_data);
        return PLUGIN_ERROR_OUT_OF_MEMORY;
    }
    
    // Decode to full size
    if (!DecodeToPixels((const char*)data, data_size, img_width, img_height, full_pixels, full_stride)) {
        result->error_code = PLUGIN_ERROR_CORRUPTED_DATA;
        result->error_message = "Corrupted image data";
        s_allocator.free(full_pixels, s_allocator.user_data);
        if (request->file_path) s_allocator.free((void*)data, s_allocator.user_data);
        return PLUGIN_ERROR_CORRUPTED_DATA;
    }
    
    if (progress) progress(0.7f, request->user_data);
    
    // Calculate scaled size
    uint32_t out_width = request->target_width;
    uint32_t out_height = request->target_height;
    
    if (request->preserve_aspect_ratio) {
        float aspect = (float)img_width / img_height;
        if (out_width / aspect > out_height) {
            out_width = (uint32_t)(out_height * aspect);
        } else {
            out_height = (uint32_t)(out_width / aspect);
        }
    }
    
    // Allocate output buffer
    uint32_t out_stride = out_width * 4;
    size_t out_size = out_stride * out_height;
    uint8_t* out_pixels = (uint8_t*)s_allocator.alloc(out_size, s_allocator.user_data);
    if (!out_pixels) {
        result->error_code = PLUGIN_ERROR_OUT_OF_MEMORY;
        result->error_message = "Out of memory";
        s_allocator.free(full_pixels, s_allocator.user_data);
        if (request->file_path) s_allocator.free((void*)data, s_allocator.user_data);
        return PLUGIN_ERROR_OUT_OF_MEMORY;
    }
    
    // Scale image
    if (out_width == img_width && out_height == img_height) {
        // No scaling needed
        memcpy(out_pixels, full_pixels, full_size);
    } else {
        ScaleImage(full_pixels, img_width, img_height, full_stride,
                  out_pixels, out_width, out_height, out_stride);
    }
    
    if (progress) progress(0.9f, request->user_data);
    
    // Free temporary buffers
    s_allocator.free(full_pixels, s_allocator.user_data);
    if (request->file_path) s_allocator.free((void*)data, s_allocator.user_data);
    
    // Fill result
    result->pixels = out_pixels;
    result->buffer_size = out_size;
    result->stride = out_stride;
    result->width = out_width;
    result->height = out_height;
    result->pixel_format = PIXEL_FORMAT_BGRA32;
    result->metadata = nullptr;
    result->error_code = PLUGIN_SUCCESS;
    result->error_message = nullptr;
    
    if (progress) progress(1.0f, request->user_data);
    
    return PLUGIN_SUCCESS;
}

PLUGIN_API void PLUGIN_CALL plugin_free_result(DecodeResult* result) {
    if (!result) return;
    
    if (result->pixels) {
        s_allocator.free(result->pixels, s_allocator.user_data);
        result->pixels = nullptr;
    }
    
    if (result->metadata) {
        s_allocator.free(result->metadata, s_allocator.user_data);
        result->metadata = nullptr;
    }
}


