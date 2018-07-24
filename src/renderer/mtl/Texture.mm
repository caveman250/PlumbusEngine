#include "renderer/mtl/Texture.h"
#include "renderer/mtl/MetalRenderer.h"

#import "Appkit/Appkit.h"
#import "Metal/Metal.h"

#include "Application.h"

@interface TextureObjc : NSObject
{
@public id<MTLTexture> m_Texture;
@public id<MTLSamplerState> m_SamplerState;
}


-(NSImage*) GetImage:(NSString*)path;
-(uint8_t*) DataForImage:(NSImage*)image;
-(void)GenerateMipmapsForTexture:(id<MTLTexture>)texture onQueue:(id<MTLCommandQueue>)queue;

@end

@implementation TextureObjc

-(uint8_t*)DataForImage:(NSImage*)image
{
    CGImageRef imageRef = [image CGImageForProposedRect:nil context: nil hints: nil];
    
    // Create a suitable bitmap context for extracting the bits of the image
    const NSUInteger width = CGImageGetWidth(imageRef);
    const NSUInteger height = CGImageGetHeight(imageRef);
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    uint8_t *rawData = (uint8_t *)calloc(height * width * 4, sizeof(uint8_t));
    const NSUInteger bytesPerPixel = 4;
    const NSUInteger bytesPerRow = bytesPerPixel * width;
    const NSUInteger bitsPerComponent = 8;
    CGContextRef context = CGBitmapContextCreate(rawData, width, height,
                                                 bitsPerComponent, bytesPerRow, colorSpace,
                                                 kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big);
    CGColorSpaceRelease(colorSpace);
    
    CGContextTranslateCTM(context, 0, height);
    CGContextScaleCTM(context, 1, -1);
    
    CGRect imageRect = CGRectMake(0, 0, width, height);
    CGContextDrawImage(context, imageRect, imageRef);
    
    CGContextRelease(context);
    
    return rawData;
}

-(NSImage *)GetImage:(NSString *)path {
    NSArray *imageReps = [NSBitmapImageRep imageRepsWithContentsOfFile:path];
    NSInteger width = 0;
    NSInteger height = 0;
    for (NSImageRep * imageRep in imageReps) {
        if ([imageRep pixelsWide] > width) width = [imageRep pixelsWide];
        if ([imageRep pixelsHigh] > height) height = [imageRep pixelsHigh];
    }
    NSImage *imageNSImage = [[NSImage alloc] initWithSize:NSMakeSize((CGFloat)width, (CGFloat)height)];
    [imageNSImage addRepresentations:imageReps];
    return imageNSImage;
}

-(void)GenerateMipmapsForTexture:(id<MTLTexture>)texture onQueue:(id<MTLCommandQueue>)queue
{
    id<MTLCommandBuffer> commandBuffer = [queue commandBuffer];
    id<MTLBlitCommandEncoder> blitEncoder = [commandBuffer blitCommandEncoder];
    [blitEncoder generateMipmapsForTexture:texture];
    [blitEncoder endEncoding];
    [commandBuffer commit];
    
    [commandBuffer waitUntilCompleted];
}


@end

namespace mtl
{
    Texture::Texture()
    {
        m_ObjCManager = [TextureObjc new];
    }
    
    void Texture::LoadTexture(std::string fileName)
    {
        MetalRenderer* renderer = static_cast<MetalRenderer*>(Application::Get().GetRenderer());
        
        NSString* nsName = [NSString stringWithCString:fileName.c_str() encoding:[NSString defaultCStringEncoding]];
        
        NSImage* image = [(TextureObjc*)m_ObjCManager GetImage:nsName];
        
        CGSize imageSize = (CGSize)image.size;
        const NSUInteger bytesPerPixel = 4;
        const NSUInteger bytesPerRow = bytesPerPixel * imageSize.width;
        uint8_t* imageData = [(TextureObjc*)m_ObjCManager DataForImage:image];
        
        MTLTextureDescriptor *textureDescriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm
                                                  width:imageSize.width
                                                  height:imageSize.height
                                                  mipmapped:true];
        
        id<MTLCommandQueue> queue = (id<MTLCommandQueue>)renderer->GetCommandQueue();
        
        ((TextureObjc*)m_ObjCManager)->m_Texture = [[queue device] newTextureWithDescriptor:textureDescriptor];
        
        MTLRegion region = MTLRegionMake2D(0, 0, imageSize.width, imageSize.height);
        [((TextureObjc*)m_ObjCManager)->m_Texture replaceRegion:region mipmapLevel:0 withBytes:imageData bytesPerRow:bytesPerRow];
        
        free(imageData);

        [(TextureObjc*)m_ObjCManager GenerateMipmapsForTexture:((TextureObjc*)m_ObjCManager)->m_Texture onQueue:queue];
        
        CreateSampler();
    }
    
    void Texture::Cleanup()
    {
        
    }
    
    void Texture::CreateSampler()
    {
        MetalRenderer* renderer = static_cast<MetalRenderer*>(Application::Get().GetRenderer());
        
        MTLSamplerDescriptor *samplerDesc = [MTLSamplerDescriptor new];
        samplerDesc.sAddressMode = MTLSamplerAddressModeClampToEdge;
        samplerDesc.tAddressMode = MTLSamplerAddressModeClampToEdge;
        samplerDesc.minFilter = MTLSamplerMinMagFilterNearest;
        samplerDesc.magFilter = MTLSamplerMinMagFilterLinear;
        samplerDesc.mipFilter = MTLSamplerMipFilterLinear;
        ((TextureObjc*)m_ObjCManager)->m_SamplerState = [(id<MTLDevice>)renderer->GetDevice() newSamplerStateWithDescriptor:samplerDesc];
    }
    
    void* Texture::GetTexture()
    {
        return ((TextureObjc*)m_ObjCManager)->m_Texture;
    }
    
    void* Texture::GetSamplerState()
    {
        return ((TextureObjc*)m_ObjCManager)->m_SamplerState;
    }
}
