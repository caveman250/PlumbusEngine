
#import "MetalView.h"

@interface MetalView ()
@property (strong) id<CAMetalDrawable> m_CurrentDrawable;
@property (assign) NSTimeInterval m_FrameDuration;
@property (strong) id<MTLTexture> depthTexture;
@property (nonatomic) id <MTLDevice> device;
@property (nonatomic) NSTimer *refreshTimer;
@end

@implementation MetalView

- (instancetype)initWithFrame:(CGRect)frame
{
    if ((self = [super initWithFrame:frame]))
    {
        [self commonInit];
    }
    
    return self;
}

-(void)setDevice:(id<MTLDevice>)device
{
    _device = device;
}

- (void)commonInit
{
     _m_FrameDuration = (1.0 / 60.0f);
    _m_ClearColour = MTLClearColorMake(0, 0, 0, 0);
    
    self.m_MetalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
}

- (CAMetalLayer *)m_MetalLayer
{
    return (CAMetalLayer *)self.layer;
}

- (CALayer *)makeBackingLayer
{
    CAMetalLayer *layer = [[CAMetalLayer alloc] init];
    layer.bounds = self.bounds;
    layer.device = self.device;
    layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    
    _layer = layer;
    return layer;
}

- (void)layout
{
    [super layout];
    CGFloat scale = [NSScreen mainScreen].backingScaleFactor;
    
    // If we've moved to a window by the time our frame is being set, we can take its scale as our own
    if (self.window)
    {
        scale = self.window.screen.backingScaleFactor;
    }
    
    CGSize drawableSize = self.bounds.size;
    
    // Since drawable size is in pixels, we need to multiply by the scale to move from points to pixels
    drawableSize.width *= scale;
    drawableSize.height *= scale;
    
    self.m_MetalLayer.drawableSize = drawableSize;
    
    [self makeDepthTextureForLayer:self.m_MetalLayer];
}

- (void)StartDrawing
{
    if (self.window)
    {
        [self.refreshTimer invalidate];
        self.refreshTimer = [NSTimer scheduledTimerWithTimeInterval:self.m_FrameDuration
                                                             target:self
                                                           selector:@selector(Redraw)
                                                           userInfo:nil
                                                            repeats:YES];
    }
}

- (void)StopDrawing
{
    [self.refreshTimer invalidate];
    self.refreshTimer = nil;
}

- (void)Redraw
{
    self.m_CurrentDrawable = [self.m_MetalLayer nextDrawable];
    
    if ([self.m_Delegate respondsToSelector:@selector(DrawInView:)])
    {
        [self.m_Delegate DrawInView:self];
    }
}

- (void)makeDepthTextureForLayer:(CAMetalLayer *)metalLayer
{
    CGSize drawableSize = metalLayer.drawableSize;
    if ([self.depthTexture width] != drawableSize.width ||
        [self.depthTexture height] != drawableSize.height)
    {
        MTLTextureDescriptor *desc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatDepth32Float
                                                                                        width:drawableSize.width
                                                                                       height:drawableSize.height
                                                                                    mipmapped:NO];
        desc.resourceOptions = MTLResourceStorageModePrivate;
        desc.usage = MTLTextureUsageRenderTarget;
        
        self.depthTexture = [metalLayer.device newTextureWithDescriptor:desc];
    }
    
    NSAssert(self.depthTexture, @"missing texture");
}

- (MTLRenderPassDescriptor *)m_CurrentRenderPassDescriptor
{
    MTLRenderPassDescriptor *passDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
    
    id <MTLTexture> texture = [self.m_CurrentDrawable texture];
    passDescriptor.colorAttachments[0].texture = texture;
    passDescriptor.colorAttachments[0].clearColor = self.m_ClearColour;
    passDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
    passDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
    
    passDescriptor.depthAttachment.texture = self.depthTexture;
    passDescriptor.depthAttachment.clearDepth = 1.0;
    passDescriptor.depthAttachment.loadAction = MTLLoadActionClear;
    passDescriptor.depthAttachment.storeAction = MTLStoreActionDontCare;
    
    return passDescriptor;
}

@end

