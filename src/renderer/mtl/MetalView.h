#import <AppKit/AppKit.h>
#import <Metal/Metal.h>
#import <QuartzCore/QuartzCore.h>

@protocol MetalViewDelegate;

@interface MetalView : NSView

@property (nonatomic, strong) id<MetalViewDelegate> m_Delegate;
@property (nonatomic, readonly) CAMetalLayer* m_MetalLayer;
@property (nonatomic, assign) MTLClearColor m_ClearColour;
@property (nonatomic, readonly) NSTimeInterval m_FrameDuration;
@property (nonatomic, readonly) id<CAMetalDrawable> m_CurrentDrawable;
@property (nonatomic, readonly) MTLRenderPassDescriptor* m_CurrentRenderPassDescriptor;

- (void)StartDrawing;
- (void)StopDrawing;

@end

@protocol MetalViewDelegate <NSObject>
- (void)DrawInView:(MetalView *)view;
@end
