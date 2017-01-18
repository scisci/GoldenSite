//
//  cg_jpeg_resizer.h
//  GoldenSite
//
//  Created by Daniel Riley on 1/6/13.
//
//

#ifndef __GoldenSite__cg_jpeg_resizer__
#define __GoldenSite__cg_jpeg_resizer__

#include <iostream>
#include <CoreFoundation/CoreFoundation.h>
#include <ApplicationServices/ApplicationServices.h>

class CGImageUtils {
public:
  static void Resize(const char* src_path, const char* dst_path, int width, int height, double quality)
  {
    
    CFURLRef src_url = CFURLCreateFromFileSystemRepresentation(0L, (const unsigned char*)src_path, strlen(src_path), false);
    CFURLRef dst_url = CFURLCreateFromFileSystemRepresentation(0L, (const unsigned char*)dst_path, strlen(dst_path), false);
    CGImageSourceRef image_src = CGImageSourceCreateWithURL(src_url, 0L);
    CGImageDestinationRef image_dst = CGImageDestinationCreateWithURL(dst_url, kUTTypeJPEG, 1, 0L);
    
    
    CGImageRef cg_image_src = CGImageSourceCreateImageAtIndex(image_src, 0, 0L);
    
    
    size_t component_bits = CGImageGetBitsPerComponent(cg_image_src);
    size_t row_bytes = width * CGImageGetBitsPerPixel(cg_image_src) / 8;
    CGColorSpaceRef colorspace = CGColorSpaceCreateWithName(kCGColorSpaceSRGB);//CGImageGetColorSpace(cg_image_src);
    CGContextRef bitmap_ctx = CGBitmapContextCreate(0L,
                                                    width,
                                                    height,
                                                    component_bits,
                                                    row_bytes,
                                                    colorspace,
                                                    CGImageGetAlphaInfo(cg_image_src) | CGImageGetBitmapInfo(cg_image_src));
    
    
    CGContextDrawImage(bitmap_ctx, CGRectMake(0, 0, width, height), cg_image_src);
    

    CGImageRef cg_image_dst = CGBitmapContextCreateImage(bitmap_ctx);

    CFMutableDictionaryRef options = CFDictionaryCreateMutable(0L, 10,  &kCFTypeDictionaryKeyCallBacks,  &kCFTypeDictionaryValueCallBacks);
    CFDictionarySetValue(options, kCGImageDestinationLossyCompressionQuality, CFNumberCreate(0L, kCFNumberDoubleType, &quality));

    CGImageDestinationAddImage(image_dst, cg_image_dst, options);
    bool result = CGImageDestinationFinalize(image_dst);
    assert(result);
    
    
    CFRelease(options);
    CFRelease(bitmap_ctx);
    CFRelease(image_src);
    CFRelease(image_dst);
    CFRelease(cg_image_src);
    CFRelease(cg_image_dst);
    CFRelease(colorspace);
    CFRelease(src_url);
    CFRelease(dst_url);
  }

};

#endif /* defined(__GoldenSite__cg_jpeg_resizer__) */
