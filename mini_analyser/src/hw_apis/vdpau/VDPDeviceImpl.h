#ifndef H_VDPDeviceImpl
#define H_VDPDeviceImpl

#include <cassert>

struct VDPDeviceImpl
{
    VDPDeviceImpl(VdpDevice dev, VdpGetProcAddress *get_proc_address);
    VdpDevice device;

    VdpGetErrorString *GetErrorString;
    VdpGetProcAddress *GetProcAddress;
    VdpGetApiVersion *GetApiVersion;
    VdpGetInformationString *GetInformationString;
    VdpDeviceDestroy *DeviceDestroy;
    VdpGenerateCSCMatrix *GenerateCSCMatrix;
    VdpVideoSurfaceQueryCapabilities *VideoSurfaceQueryCapabilities;
    VdpVideoSurfaceQueryGetPutBitsYCbCrCapabilities *VideoSurfaceQueryGetPutBitsYCbCrCapabilities;
    VdpVideoSurfaceCreate *VideoSurfaceCreate;
    VdpVideoSurfaceDestroy *VideoSurfaceDestroy;
    VdpVideoSurfaceGetParameters *VideoSurfaceGetParameters;
    VdpVideoSurfaceGetBitsYCbCr *VideoSurfaceGetBitsYCbCr;
    VdpVideoSurfacePutBitsYCbCr *VideoSurfacePutBitsYCbCr;
    VdpOutputSurfaceQueryCapabilities *OutputSurfaceQueryCapabilities;
    VdpOutputSurfaceQueryGetPutBitsNativeCapabilities *OutputSurfaceQueryGetPutBitsNativeCapabilities;
    VdpOutputSurfaceQueryPutBitsIndexedCapabilities *OutputSurfaceQueryPutBitsIndexedCapabilities;
    VdpOutputSurfaceQueryPutBitsYCbCrCapabilities *OutputSurfaceQueryPutBitsYCbCrCapabilities;
    VdpOutputSurfaceCreate *OutputSurfaceCreate;
    VdpOutputSurfaceDestroy *OutputSurfaceDestroy;
    VdpOutputSurfaceGetParameters *OutputSurfaceGetParameters;
    VdpOutputSurfaceGetBitsNative *OutputSurfaceGetBitsNative;
    VdpOutputSurfacePutBitsNative *OutputSurfacePutBitsNative;
    VdpOutputSurfacePutBitsIndexed *OutputSurfacePutBitsIndexed;
    VdpOutputSurfacePutBitsYCbCr *OutputSurfacePutBitsYCbCr;
    VdpBitmapSurfaceQueryCapabilities *BitmapSurfaceQueryCapabilities;
    VdpBitmapSurfaceCreate *BitmapSurfaceCreate;
    VdpBitmapSurfaceDestroy *BitmapSurfaceDestroy;
    VdpBitmapSurfaceGetParameters *BitmapSurfaceGetParameters;
    VdpBitmapSurfacePutBitsNative *BitmapSurfacePutBitsNative;
    VdpOutputSurfaceRenderOutputSurface *OutputSurfaceRenderOutputSurface;
    VdpOutputSurfaceRenderBitmapSurface *OutputSurfaceRenderBitmapSurface;
    VdpDecoderQueryCapabilities *DecoderQueryCapabilities;
    VdpDecoderCreate *DecoderCreate;
    VdpDecoderDestroy *DecoderDestroy;
    VdpDecoderGetParameters *DecoderGetParameters;
    VdpDecoderRender *DecoderRender;
    VdpVideoMixerQueryFeatureSupport *VideoMixerQueryFeatureSupport;
    VdpVideoMixerQueryParameterSupport *VideoMixerQueryParameterSupport;
    VdpVideoMixerQueryAttributeSupport *VideoMixerQueryAttributeSupport;
    VdpVideoMixerQueryParameterValueRange *VideoMixerQueryParameterValueRange;
    VdpVideoMixerQueryAttributeValueRange *VideoMixerQueryAttributeValueRange;
    VdpVideoMixerCreate *VideoMixerCreate;
    VdpVideoMixerSetFeatureEnables *VideoMixerSetFeatureEnables;
    VdpVideoMixerSetAttributeValues *VideoMixerSetAttributeValues;
    VdpVideoMixerGetFeatureSupport *VideoMixerGetFeatureSupport;
    VdpVideoMixerGetFeatureEnables *VideoMixerGetFeatureEnables;
    VdpVideoMixerGetParameterValues *VideoMixerGetParameterValues;
    VdpVideoMixerGetAttributeValues *VideoMixerGetAttributeValues;
    VdpVideoMixerDestroy *VideoMixerDestroy;
    VdpVideoMixerRender *VideoMixerRender;
    VdpPresentationQueueTargetDestroy *PresentationQueueTargetDestroy;
    VdpPresentationQueueCreate *PresentationQueueCreate;
    VdpPresentationQueueDestroy *PresentationQueueDestroy;
    VdpPresentationQueueSetBackgroundColor *PresentationQueueSetBackgroundColor;
    VdpPresentationQueueGetBackgroundColor *PresentationQueueGetBackgroundColor;
    VdpPresentationQueueGetTime *PresentationQueueGetTime;
    VdpPresentationQueueDisplay *PresentationQueueDisplay;
    VdpPresentationQueueBlockUntilSurfaceIdle *PresentationQueueBlockUntilSurfaceIdle;
    VdpPresentationQueueQuerySurfaceStatus *PresentationQueueQuerySurfaceStatus;
    VdpPreemptionCallbackRegister *PreemptionCallbackRegister;
    VdpPresentationQueueTargetCreateX11 *PresentationQueueTargetCreateX11;
};

#define GETADDR(device, function_id, function_pointer) \
    assert(get_proc_address(device, function_id, function_pointer) == VDP_STATUS_OK)

VDPDeviceImpl::VDPDeviceImpl(VdpDevice dev, VdpGetProcAddress *get_proc_address):
    device(dev)
{
    GETADDR(dev, VDP_FUNC_ID_GET_ERROR_STRING, (void**)&GetErrorString);
    GETADDR(dev, VDP_FUNC_ID_GET_PROC_ADDRESS, (void**)&GetProcAddress);
    GETADDR(dev, VDP_FUNC_ID_GET_API_VERSION, (void**)&GetApiVersion);
    GETADDR(dev, VDP_FUNC_ID_GET_INFORMATION_STRING, (void**)&GetInformationString);
    GETADDR(dev, VDP_FUNC_ID_DEVICE_DESTROY, (void**)&DeviceDestroy);
    GETADDR(dev, VDP_FUNC_ID_GENERATE_CSC_MATRIX, (void**)&GenerateCSCMatrix);
    GETADDR(dev, VDP_FUNC_ID_VIDEO_SURFACE_QUERY_CAPABILITIES, (void**)&VideoSurfaceQueryCapabilities);
    GETADDR(dev, VDP_FUNC_ID_VIDEO_SURFACE_QUERY_GET_PUT_BITS_Y_CB_CR_CAPABILITIES, (void**)&VideoSurfaceQueryGetPutBitsYCbCrCapabilities);
    GETADDR(dev, VDP_FUNC_ID_VIDEO_SURFACE_CREATE, (void**)&VideoSurfaceCreate);
    GETADDR(dev, VDP_FUNC_ID_VIDEO_SURFACE_DESTROY, (void**)&VideoSurfaceDestroy);
    GETADDR(dev, VDP_FUNC_ID_VIDEO_SURFACE_GET_PARAMETERS, (void**)&VideoSurfaceGetParameters);
    GETADDR(dev, VDP_FUNC_ID_VIDEO_SURFACE_GET_BITS_Y_CB_CR, (void**)&VideoSurfaceGetBitsYCbCr);
    GETADDR(dev, VDP_FUNC_ID_VIDEO_SURFACE_PUT_BITS_Y_CB_CR, (void**)&VideoSurfacePutBitsYCbCr);
    GETADDR(dev, VDP_FUNC_ID_OUTPUT_SURFACE_QUERY_CAPABILITIES, (void**)&OutputSurfaceQueryCapabilities);
    GETADDR(dev, VDP_FUNC_ID_OUTPUT_SURFACE_QUERY_GET_PUT_BITS_NATIVE_CAPABILITIES, (void**)&OutputSurfaceQueryGetPutBitsNativeCapabilities);
    GETADDR(dev, VDP_FUNC_ID_OUTPUT_SURFACE_QUERY_PUT_BITS_INDEXED_CAPABILITIES, (void**)&OutputSurfaceQueryPutBitsIndexedCapabilities);
    GETADDR(dev, VDP_FUNC_ID_OUTPUT_SURFACE_QUERY_PUT_BITS_Y_CB_CR_CAPABILITIES, (void**)&OutputSurfaceQueryPutBitsYCbCrCapabilities);
    GETADDR(dev, VDP_FUNC_ID_OUTPUT_SURFACE_CREATE, (void**)&OutputSurfaceCreate);
    GETADDR(dev, VDP_FUNC_ID_OUTPUT_SURFACE_DESTROY, (void**)&OutputSurfaceDestroy);
    GETADDR(dev, VDP_FUNC_ID_OUTPUT_SURFACE_GET_PARAMETERS, (void**)&OutputSurfaceGetParameters);
    GETADDR(dev, VDP_FUNC_ID_OUTPUT_SURFACE_GET_BITS_NATIVE, (void**)&OutputSurfaceGetBitsNative);
    GETADDR(dev, VDP_FUNC_ID_OUTPUT_SURFACE_PUT_BITS_NATIVE, (void**)&OutputSurfacePutBitsNative);
    GETADDR(dev, VDP_FUNC_ID_OUTPUT_SURFACE_PUT_BITS_INDEXED, (void**)&OutputSurfacePutBitsIndexed);
    GETADDR(dev, VDP_FUNC_ID_OUTPUT_SURFACE_PUT_BITS_Y_CB_CR, (void**)&OutputSurfacePutBitsYCbCr);
    GETADDR(dev, VDP_FUNC_ID_BITMAP_SURFACE_QUERY_CAPABILITIES, (void**)&BitmapSurfaceQueryCapabilities);
    GETADDR(dev, VDP_FUNC_ID_BITMAP_SURFACE_CREATE, (void**)&BitmapSurfaceCreate);
    GETADDR(dev, VDP_FUNC_ID_BITMAP_SURFACE_DESTROY, (void**)&BitmapSurfaceDestroy);
    GETADDR(dev, VDP_FUNC_ID_BITMAP_SURFACE_GET_PARAMETERS, (void**)&BitmapSurfaceGetParameters);
    GETADDR(dev, VDP_FUNC_ID_BITMAP_SURFACE_PUT_BITS_NATIVE, (void**)&BitmapSurfacePutBitsNative);
    GETADDR(dev, VDP_FUNC_ID_OUTPUT_SURFACE_RENDER_OUTPUT_SURFACE, (void**)&OutputSurfaceRenderOutputSurface);
    GETADDR(dev, VDP_FUNC_ID_OUTPUT_SURFACE_RENDER_BITMAP_SURFACE, (void**)&OutputSurfaceRenderBitmapSurface);
    GETADDR(dev, VDP_FUNC_ID_DECODER_QUERY_CAPABILITIES, (void**)&DecoderQueryCapabilities);
    GETADDR(dev, VDP_FUNC_ID_DECODER_CREATE, (void**)&DecoderCreate);
    GETADDR(dev, VDP_FUNC_ID_DECODER_DESTROY, (void**)&DecoderDestroy);
    GETADDR(dev, VDP_FUNC_ID_DECODER_GET_PARAMETERS, (void**)&DecoderGetParameters);
    GETADDR(dev, VDP_FUNC_ID_DECODER_RENDER, (void**)&DecoderRender);
    GETADDR(dev, VDP_FUNC_ID_VIDEO_MIXER_QUERY_FEATURE_SUPPORT, (void**)&VideoMixerQueryFeatureSupport);
    GETADDR(dev, VDP_FUNC_ID_VIDEO_MIXER_QUERY_PARAMETER_SUPPORT, (void**)&VideoMixerQueryParameterSupport);
    GETADDR(dev, VDP_FUNC_ID_VIDEO_MIXER_QUERY_ATTRIBUTE_SUPPORT, (void**)&VideoMixerQueryAttributeSupport);
    GETADDR(dev, VDP_FUNC_ID_VIDEO_MIXER_QUERY_PARAMETER_VALUE_RANGE, (void**)&VideoMixerQueryParameterValueRange);
    GETADDR(dev, VDP_FUNC_ID_VIDEO_MIXER_QUERY_ATTRIBUTE_VALUE_RANGE, (void**)&VideoMixerQueryAttributeValueRange);
    GETADDR(dev, VDP_FUNC_ID_VIDEO_MIXER_CREATE, (void**)&VideoMixerCreate);
    GETADDR(dev, VDP_FUNC_ID_VIDEO_MIXER_SET_FEATURE_ENABLES, (void**)&VideoMixerSetFeatureEnables);
    GETADDR(dev, VDP_FUNC_ID_VIDEO_MIXER_SET_ATTRIBUTE_VALUES, (void**)&VideoMixerSetAttributeValues);
    GETADDR(dev, VDP_FUNC_ID_VIDEO_MIXER_GET_FEATURE_SUPPORT, (void**)&VideoMixerGetFeatureSupport);
    GETADDR(dev, VDP_FUNC_ID_VIDEO_MIXER_GET_FEATURE_ENABLES, (void**)&VideoMixerGetFeatureEnables);
    GETADDR(dev, VDP_FUNC_ID_VIDEO_MIXER_GET_PARAMETER_VALUES, (void**)&VideoMixerGetParameterValues);
    GETADDR(dev, VDP_FUNC_ID_VIDEO_MIXER_GET_ATTRIBUTE_VALUES, (void**)&VideoMixerGetAttributeValues);
    GETADDR(dev, VDP_FUNC_ID_VIDEO_MIXER_DESTROY, (void**)&VideoMixerDestroy);
    GETADDR(dev, VDP_FUNC_ID_VIDEO_MIXER_RENDER, (void**)&VideoMixerRender);
    GETADDR(dev, VDP_FUNC_ID_PRESENTATION_QUEUE_TARGET_DESTROY, (void**)&PresentationQueueTargetDestroy);
    GETADDR(dev, VDP_FUNC_ID_PRESENTATION_QUEUE_CREATE, (void**)&PresentationQueueCreate);
    GETADDR(dev, VDP_FUNC_ID_PRESENTATION_QUEUE_DESTROY, (void**)&PresentationQueueDestroy);
    GETADDR(dev, VDP_FUNC_ID_PRESENTATION_QUEUE_SET_BACKGROUND_COLOR, (void**)&PresentationQueueSetBackgroundColor);
    GETADDR(dev, VDP_FUNC_ID_PRESENTATION_QUEUE_GET_BACKGROUND_COLOR, (void**)&PresentationQueueGetBackgroundColor);
    GETADDR(dev, VDP_FUNC_ID_PRESENTATION_QUEUE_GET_TIME, (void**)&PresentationQueueGetTime);
    GETADDR(dev, VDP_FUNC_ID_PRESENTATION_QUEUE_DISPLAY, (void**)&PresentationQueueDisplay);
    GETADDR(dev, VDP_FUNC_ID_PRESENTATION_QUEUE_BLOCK_UNTIL_SURFACE_IDLE, (void**)&PresentationQueueBlockUntilSurfaceIdle);
    GETADDR(dev, VDP_FUNC_ID_PRESENTATION_QUEUE_QUERY_SURFACE_STATUS, (void**)&PresentationQueueQuerySurfaceStatus);
    GETADDR(dev, VDP_FUNC_ID_PREEMPTION_CALLBACK_REGISTER, (void**)&PreemptionCallbackRegister);
    GETADDR(dev, VDP_FUNC_ID_PRESENTATION_QUEUE_TARGET_CREATE_X11, (void**)&PresentationQueueTargetCreateX11);
}
#undef GETADDR

#endif // H_VDPDeviceImpl
