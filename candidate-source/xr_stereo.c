/* Fresh official-SDK consumer for the native-camera experiment. The pair's
 * original camera clocks remain distinct and are never called one sim tick. */
#define COBJMACROS
#define WIN32_LEAN_AND_MEAN
#define XR_NO_PROTOTYPES
#define XR_USE_GRAPHICS_API_D3D11
#include <windows.h>
#include <mmsystem.h>
#include <d3d11.h>
#include <dxgi.h>
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>
#include <openxr/openxr_loader_negotiation.h>
#include "stereo_wire.h"
#include "openxr_bindings.h"
#include "xr_hands_export.h"
#include "xr_finger_touch.h"
#include "xr_haptic_feedback.h"
#include "xr_driver_diagnostics.h"
#include "ui_projection.h"
#include "xr_smaa.h"
int _fltused=0;
void* __cdecl memset(void*p,int c,size_t n){volatile unsigned char*q=p;for(size_t k=0;k<n;k++)q[k]=(unsigned char)c;return p;}
#pragma optimize("", off)
void* __cdecl memcpy(void*p,const void*s,size_t n){unsigned char*d=p;const unsigned char*q=s;for(size_t k=0;k<n;k++)d[k]=q[k];return p;}
#pragma optimize("", on)
static void out(const char*s){DWORD n=0,w;while(s[n])n++;WriteFile(GetStdHandle(STD_OUTPUT_HANDLE),s,n,&w,0);}
static void number(DWORD value){
 char digits[10];
 char *end=digits+sizeof(digits), *first=end;
 DWORD written;
 do{*--first=(char)('0'+value%10);value/=10;}while(value);
 WriteFile(GetStdHandle(STD_OUTPUT_HANDLE),first,(DWORD)(end-first),&written,0);
}
static void signed_number(XrResult v){if(v<0){out("-");number(0u-(DWORD)v);}else number((DWORD)v);}
static const char*xr_result_name(XrResult v){switch(v){
 case XR_ERROR_FORM_FACTOR_UNAVAILABLE:return "XR_ERROR_FORM_FACTOR_UNAVAILABLE";
 case XR_ERROR_FORM_FACTOR_UNSUPPORTED:return "XR_ERROR_FORM_FACTOR_UNSUPPORTED";
 case XR_ERROR_RUNTIME_UNAVAILABLE:return "XR_ERROR_RUNTIME_UNAVAILABLE";
 case XR_ERROR_RUNTIME_FAILURE:return "XR_ERROR_RUNTIME_FAILURE";
 case XR_ERROR_INITIALIZATION_FAILED:return "XR_ERROR_INITIALIZATION_FAILED";
 case XR_ERROR_INSTANCE_LOST:return "XR_ERROR_INSTANCE_LOST";
 case XR_ERROR_EXTENSION_NOT_PRESENT:return "XR_ERROR_EXTENSION_NOT_PRESENT";
 default:return "XR_ERROR_OTHER";
}}
static void xr_failure(const char*call,XrResult v){out("XR_FAIL ");out(call);out(" result=");signed_number(v);out(" name=");out(xr_result_name(v));out("\r\n");}
static BOOL startup_stop_requested(void){WCHAR path[1024];DWORD n=GetEnvironmentVariableW(L"AC1_ENGINE_STOP_FILE",path,1024);return n&&n<1024&&GetFileAttributesW(path)!=INVALID_FILE_ATTRIBUTES;}
static PFN_xrGetInstanceProcAddr runtime_gp,layer_gp;static PFN_xrCreateInstance runtime_create;
static PFN_xrCreateApiLayerInstance layer_create;
static XrResult XRAPI_CALL next_create(const XrInstanceCreateInfo*i,const XrApiLayerCreateInfo*l,XrInstance*r){(void)l;return runtime_create(i,r);}
static XrResult create_taped(const XrInstanceCreateInfo*i,XrInstance*r){
 XrApiLayerNextInfo ni={XR_LOADER_INTERFACE_STRUCT_API_LAYER_NEXT_INFO,XR_API_LAYER_NEXT_INFO_STRUCT_VERSION,sizeof(ni)};
 memcpy(ni.layerName,"XR_APILAYER_XRTAPE_recorder",sizeof("XR_APILAYER_XRTAPE_recorder"));ni.nextGetInstanceProcAddr=runtime_gp;ni.nextCreateApiLayerInstance=next_create;
 XrApiLayerCreateInfo li={XR_LOADER_INTERFACE_STRUCT_API_LAYER_CREATE_INFO,XR_API_LAYER_CREATE_INFO_STRUCT_VERSION,sizeof(li)};li.nextInfo=&ni;
 return layer_create(i,&li,r);
}
static const GUID factory_id={0x7b7166ec,0x21c7,0x44ae,{0xb2,0x1a,0xc9,0xae,0x32,0x1a,0xe3,0x69}};
static const GUID texture_id={0x6f15aaf2,0xd208,0x4e89,{0x9a,0xb4,0x48,0x95,0x35,0xd3,0x4f,0x9c}};
static StereoWire*wire;
static Ac1FingerTouch finger_touch;
static Ac1Haptic haptic;
static Ac1HandsExporter hands;static Ac1HandsWire*hands_view;static HANDLE hands_mapping;
static XrAction controls[12];
static ID3D11Device*device;static ID3D11DeviceContext*context;static ID3D11Texture2D*source[2],*stage[2];
static ID3D11Texture2D*menu_source;
static ID3D11Query*query;
static void note_gpu_wait(LARGE_INTEGER start){
 if(!wire)return;LARGE_INTEGER now,f;QueryPerformanceCounter(&now);QueryPerformanceFrequency(&f);
 DWORD us=(DWORD)((double)(now.QuadPart-start.QuadPart)*1000000.0/(double)f.QuadPart);
 wire->consumer_waits++;wire->consumer_wait_total_us+=us;if(us>wire->consumer_wait_max_us)wire->consumer_wait_max_us=us;
}
static int wait_gpu(void){
 LARGE_INTEGER wait_start;QueryPerformanceCounter(&wait_start);
 ID3D11DeviceContext_End(context,(ID3D11Asynchronous*)query);ID3D11DeviceContext_Flush(context);DWORD start=GetTickCount();BOOL done=FALSE;HRESULT h;
 do{h=ID3D11DeviceContext_GetData(context,(ID3D11Asynchronous*)query,&done,sizeof(done),0);if(h==S_OK&&done){note_gpu_wait(wait_start);return 1;}if(FAILED(h)){out("XR_GPU_WAIT_FAILED HRESULT=");number((DWORD)h);out(" DEVICE_REASON=");number((DWORD)ID3D11Device_GetDeviceRemovedReason(device));out("\r\n");note_gpu_wait(wait_start);return 0;}Sleep(1);}while(GetTickCount()-start<2000);out("XR_GPU_WAIT_TIMEOUT_MS=2000\r\n");note_gpu_wait(wait_start);return 0;
}
static int verify_transfer(ID3D11Texture2D*a,ID3D11Texture2D*b,DXGI_FORMAT format,DWORD*hash){
 ID3D11DeviceContext_CopyResource(context,(ID3D11Resource*)stage[0],(ID3D11Resource*)a);
 ID3D11DeviceContext_CopyResource(context,(ID3D11Resource*)stage[1],(ID3D11Resource*)b);
 if(!wait_gpu())return 0;D3D11_MAPPED_SUBRESOURCE ma={0},mb={0};
 if(FAILED(ID3D11DeviceContext_Map(context,(ID3D11Resource*)stage[0],0,D3D11_MAP_READ,0,&ma)))return 0;
 if(FAILED(ID3D11DeviceContext_Map(context,(ID3D11Resource*)stage[1],0,D3D11_MAP_READ,0,&mb))){ID3D11DeviceContext_Unmap(context,(ID3D11Resource*)stage[0],0);return 0;}
 DWORD hv=2166136261u;int equal=1;
 for(DWORD y=0;y<wire->height;y++){
  const unsigned char*pa=(const unsigned char*)ma.pData+y*ma.RowPitch,*pb=(const unsigned char*)mb.pData+y*mb.RowPitch;
  if(!display_transfer_matches(format,pa,pb,wire->width,&hv))equal=0;
 }
 ID3D11DeviceContext_Unmap(context,(ID3D11Resource*)stage[1],0);ID3D11DeviceContext_Unmap(context,(ID3D11Resource*)stage[0],0);*hash=hv;return equal;
}
#define GET(name) PFN_##name name=0;if(gp(inst,#name,(PFN_xrVoidFunction*)&name)<0||!name){code=11;goto done;}
#define TRY(expr) do{rd_enter(#expr);XrResult xr_result=(expr);rd_leave(xr_result);if(XR_FAILED(xr_result)){xr_failure(#expr,xr_result);code=12;goto done;}}while(0)
int main(void){
 rd_init();rd_enter("XR_DRIVER_STARTUP");rd_leave(0);
 DisplayPipeline display={0};
 SmaaPipeline smaa={0};
 BOOL smaaReady=FALSE;
 DWORD code=0;BOOL timer_active=FALSE;static WCHAR runtime_path[600],tape_path[600],mapname[160];HANDLE mapping=0;HMODULE runtime=0,tape=0;XrInstance inst=XR_NULL_HANDLE;XrSession session=XR_NULL_HANDLE;XrSpace space=XR_NULL_HANDLE,view_space=XR_NULL_HANDLE;XrSwapchain swaps[2]={0,0},menu_swap=XR_NULL_HANDLE;XrActionSet action_set=XR_NULL_HANDLE;XrAction confirm_action=XR_NULL_HANDLE;
 PFN_xrDestroyActionSet destroy_action_set=0;
 PFN_xrApplyHapticFeedback apply_haptic=0;PFN_xrStopHapticFeedback stop_haptic=0;
 PFN_xrDestroyInstance destroy_instance=0;PFN_xrDestroySession destroy_session=0;PFN_xrDestroySpace destroy_space=0;PFN_xrDestroySwapchain destroy_swap=0;
 IDXGIFactory*factory=0;IDXGIAdapter*adapter=0;PFN_xrGetInstanceProcAddr gp=0;
 XrExtensionProperties*available_extensions=0;int64_t*formats=0;
 XrSwapchainImageD3D11KHR*images[2]={0,0},*menu_images=0;
 XrAction all_actions[AC1_ACTION_COUNT]={0};BOOL profile_enabled[AC1_PROFILE_COUNT]={0};
 const char*enabled_extensions[AC1_PROFILE_COUNT+1];uint32_t enabled_count=0;
 WCHAR option[8];BOOL real=GetEnvironmentVariableW(L"AC1_ENGINE_REAL_OPENXR",option,8)&&option[0]==L'1';BOOL probe=GetEnvironmentVariableW(L"AC1_ENGINE_PROBE_ONLY",option,8)&&option[0]==L'1';BOOL actions_only=GetEnvironmentVariableW(L"AC1_ENGINE_ACTIONS_ONLY",option,8)&&option[0]==L'1';if(real&&actions_only){code=27;goto done;}
 BOOL slow_capture_lab=!real&&GetEnvironmentVariableW(L"AC1_ENGINE_SLOW_CAPTURE_LAB",option,8)&&option[0]==L'1';DWORD max_pair_age=slow_capture_lab?2000u:250u;if(slow_capture_lab)out("LAB_STAGE_READBACK_NOT_REALTIME max_pair_age_ms=2000\r\n");
 BOOL sim_present=!real&&GetEnvironmentVariableW(L"AC1_ENGINE_SIM_PRESENT",option,8)&&option[0]==L'1';if(sim_present)out("SIM_PRESENT_MODE startup4equalitychecks_then_GPU_fences_no_per_pair_CPU_readback\r\n");
 if(!GetEnvironmentVariableW(L"AC1_ENGINE_XRSIM_DLL",runtime_path,600)||!GetEnvironmentVariableW(L"AC1_ENGINE_TAPE_DLL",tape_path,600)||(!probe&&!GetEnvironmentVariableW(L"AC1_ENGINE_STEREO_MAP",mapname,160))){code=1;goto done;}
 runtime=LoadLibraryW(runtime_path);tape=LoadLibraryW(tape_path);if(!runtime||!tape){code=2;goto done;}
 if(real){
  runtime_gp=(PFN_xrGetInstanceProcAddr)GetProcAddress(runtime,"xrGetInstanceProcAddr");runtime_create=(PFN_xrCreateInstance)GetProcAddress(runtime,"xrCreateInstance");
  if(!runtime_gp||!runtime_create){code=3;goto done;}gp=runtime_gp;out("REAL_SYSTEM_OPENXR_LOADER\r\n");
  PFN_xrNegotiateLoaderApiLayerInterface nl=(PFN_xrNegotiateLoaderApiLayerInterface)GetProcAddress(tape,"xrNegotiateLoaderApiLayerInterface");if(!nl){code=3;goto done;}
  XrNegotiateLoaderInfo li={XR_LOADER_INTERFACE_STRUCT_LOADER_INFO,1,sizeof(li),1,1,XR_MAKE_VERSION(1,0,0),XR_CURRENT_API_VERSION};
  XrNegotiateApiLayerRequest lr={XR_LOADER_INTERFACE_STRUCT_API_LAYER_REQUEST,1,sizeof(lr)};TRY(nl(&li,"XR_APILAYER_XRTAPE_recorder",&lr));layer_gp=lr.getInstanceProcAddr;layer_create=lr.createApiLayerInstance;gp=layer_gp;
 }else{
 PFN_xrNegotiateLoaderRuntimeInterface nr=(PFN_xrNegotiateLoaderRuntimeInterface)GetProcAddress(runtime,"xrNegotiateLoaderRuntimeInterface");
 PFN_xrNegotiateLoaderApiLayerInterface nl=(PFN_xrNegotiateLoaderApiLayerInterface)GetProcAddress(tape,"xrNegotiateLoaderApiLayerInterface");if(!nr||!nl){code=3;goto done;}
 XrNegotiateLoaderInfo li={XR_LOADER_INTERFACE_STRUCT_LOADER_INFO,1,sizeof(li),1,1,XR_MAKE_VERSION(1,0,0),XR_CURRENT_API_VERSION};
 XrNegotiateRuntimeRequest rr={XR_LOADER_INTERFACE_STRUCT_RUNTIME_REQUEST,1,sizeof(rr)};TRY(nr(&li,&rr));runtime_gp=rr.getInstanceProcAddr;
 TRY(runtime_gp(XR_NULL_HANDLE,"xrCreateInstance",(PFN_xrVoidFunction*)&runtime_create));
 XrNegotiateApiLayerRequest lr={XR_LOADER_INTERFACE_STRUCT_API_LAYER_REQUEST,1,sizeof(lr)};TRY(nl(&li,"XR_APILAYER_XRTAPE_recorder",&lr));layer_gp=lr.getInstanceProcAddr;layer_create=lr.createApiLayerInstance;
 gp=layer_gp;
 }
 PFN_xrEnumerateInstanceExtensionProperties enumerate_extensions=0;
 TRY(runtime_gp(XR_NULL_HANDLE,"xrEnumerateInstanceExtensionProperties",(PFN_xrVoidFunction*)&enumerate_extensions));
 if(!enumerate_extensions){code=35;goto done;}
 uint32_t extension_count=0;TRY(enumerate_extensions(0,0,&extension_count,0));
 available_extensions=HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,(SIZE_T)extension_count*sizeof(*available_extensions));
 if(!available_extensions){code=35;goto done;}
 for(uint32_t i=0;i<extension_count;i++)available_extensions[i].type=XR_TYPE_EXTENSION_PROPERTIES;
 TRY(enumerate_extensions(0,extension_count,&extension_count,available_extensions));
 BOOL has_d3d11=FALSE;
 for(uint32_t i=0;i<extension_count;i++)if(!lstrcmpA(available_extensions[i].extensionName,XR_KHR_D3D11_ENABLE_EXTENSION_NAME))has_d3d11=TRUE;
 if(!has_d3d11){out("OPENXR_RUNTIME_MISSING_XR_KHR_D3D11_enable\r\n");code=35;goto done;}
 enabled_extensions[enabled_count++]=XR_KHR_D3D11_ENABLE_EXTENSION_NAME;
 for(unsigned p=0;p<AC1_PROFILE_COUNT;p++){
  const char*ext=ac1_profiles[p].extension;if(!ext){profile_enabled[p]=TRUE;continue;}
  for(uint32_t i=0;i<extension_count;i++)if(!lstrcmpA(available_extensions[i].extensionName,ext)){profile_enabled[p]=TRUE;break;}
  if(profile_enabled[p]){
   BOOL duplicate=FALSE;for(uint32_t i=0;i<enabled_count;i++)if(!lstrcmpA(enabled_extensions[i],ext))duplicate=TRUE;
   if(!duplicate){enabled_extensions[enabled_count++]=ext;out("XR_ENABLED_CONTROLLER_EXTENSION=");out(ext);out("\r\n");}
  }
 }
 HeapFree(GetProcessHeap(),0,available_extensions);available_extensions=0;
 XrInstanceCreateInfo ci={XR_TYPE_INSTANCE_CREATE_INFO};ci.applicationInfo.apiVersion=XR_MAKE_VERSION(1,0,0);memcpy(ci.applicationInfo.applicationName,"AC1 original camera stereo",sizeof("AC1 original camera stereo"));ci.enabledExtensionCount=enabled_count;ci.enabledExtensionNames=enabled_extensions;
 TRY(create_taped(&ci,&inst));
 gp(inst,"xrDestroyInstance",(PFN_xrVoidFunction*)&destroy_instance);
 GET(xrGetSystem);GET(xrGetSystemProperties);GET(xrGetD3D11GraphicsRequirementsKHR);GET(xrCreateSession);GET(xrCreateReferenceSpace);GET(xrPollEvent);GET(xrBeginSession);GET(xrEndSession);GET(xrWaitFrame);GET(xrBeginFrame);GET(xrLocateViews);GET(xrEndFrame);GET(xrCreateSwapchain);GET(xrEnumerateSwapchainFormats);GET(xrEnumerateSwapchainImages);GET(xrAcquireSwapchainImage);GET(xrWaitSwapchainImage);GET(xrReleaseSwapchainImage);
 GET(xrCreateActionSet);GET(xrCreateAction);GET(xrStringToPath);GET(xrPathToString);GET(xrGetCurrentInteractionProfile);GET(xrGetInstanceProperties);GET(xrSuggestInteractionProfileBindings);GET(xrAttachSessionActionSets);GET(xrSyncActions);GET(xrGetActionStateBoolean);GET(xrGetActionStateFloat);GET(xrGetActionStateVector2f);GET(xrCreateActionSpace);GET(xrGetActionStatePose);GET(xrLocateSpace);
 gp(inst,"xrApplyHapticFeedback",(PFN_xrVoidFunction*)&apply_haptic);gp(inst,"xrStopHapticFeedback",(PFN_xrVoidFunction*)&stop_haptic);
 hands.create_space=xrCreateActionSpace;hands.state_pose=xrGetActionStatePose;hands.locate=xrLocateSpace;
 gp(inst,"xrDestroyActionSet",(PFN_xrVoidFunction*)&destroy_action_set);
 gp(inst,"xrDestroySession",(PFN_xrVoidFunction*)&destroy_session);gp(inst,"xrDestroySpace",(PFN_xrVoidFunction*)&destroy_space);gp(inst,"xrDestroySwapchain",(PFN_xrVoidFunction*)&destroy_swap);
 XrInstanceProperties runtime_properties={XR_TYPE_INSTANCE_PROPERTIES};TRY(xrGetInstanceProperties(inst,&runtime_properties));
 out("XR_RUNTIME_NAME=");out(runtime_properties.runtimeName);out(" VERSION=");number(XR_VERSION_MAJOR(runtime_properties.runtimeVersion));out(".");number(XR_VERSION_MINOR(runtime_properties.runtimeVersion));out(".");number(XR_VERSION_PATCH(runtime_properties.runtimeVersion));out("\r\n");
 XrSystemGetInfo gi={XR_TYPE_SYSTEM_GET_INFO};gi.formFactor=XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;XrSystemId system=0;
 XrResult system_result=xrGetSystem(inst,&gi,&system);
 if(real&&!probe&&system_result==XR_ERROR_FORM_FACTOR_UNAVAILABLE){
  out("XR_WAITING_FOR_HEADSET_CONNECTION result=-35 name=XR_ERROR_FORM_FACTOR_UNAVAILABLE max_wait_ms=unlimited\r\n");
  DWORD system_retry_start=GetTickCount();
  while(system_result==XR_ERROR_FORM_FACTOR_UNAVAILABLE){
   if(startup_stop_requested()){out("XR_STARTUP_CANCELLED\r\n");goto done;}
   Sleep(100);if(GetTickCount()-system_retry_start>=500){system_retry_start=GetTickCount();system_result=xrGetSystem(inst,&gi,&system);}
  }
 }
 if(XR_FAILED(system_result)){xr_failure("xrGetSystem(inst,&gi,&system)",system_result);code=12;goto done;}
 XrGraphicsRequirementsD3D11KHR req={XR_TYPE_GRAPHICS_REQUIREMENTS_D3D11_KHR};TRY(xrGetD3D11GraphicsRequirementsKHR(inst,system,&req));
 XrSystemProperties properties={XR_TYPE_SYSTEM_PROPERTIES};TRY(xrGetSystemProperties(inst,system,&properties));out("ACTUAL_XR_SYSTEM=");out(properties.systemName);out(" LUID=");number(req.adapterLuid.LowPart);out("\r\n");
 XrActionSetCreateInfo asi={XR_TYPE_ACTION_SET_CREATE_INFO};memcpy(asi.actionSetName,"ac1_controls",sizeof("ac1_controls"));memcpy(asi.localizedActionSetName,"AC1 Controls",sizeof("AC1 Controls"));TRY(xrCreateActionSet(inst,&asi,&action_set));
 for(unsigned a=0;a<AC1_ACTION_COUNT;a++){
  XrActionCreateInfo ai={XR_TYPE_ACTION_CREATE_INFO};ai.actionType=ac1_action_types[a];unsigned n=0;while(ac1_action_names[a][n])n++;
  memcpy(ai.actionName,ac1_action_names[a],n+1);memcpy(ai.localizedActionName,ac1_action_names[a],n+1);TRY(xrCreateAction(action_set,&ai,&all_actions[a]));
 }
 confirm_action=all_actions[0];for(unsigned i=0;i<12;i++)controls[i]=all_actions[i+1];
 TRY(ac1_hands_create_actions(&hands,inst,action_set,xrStringToPath,xrCreateAction));
 out(ac1_fingers_create(&finger_touch,action_set,hands.paths,xrCreateAction)?"XR_FINGER_ACTIONS_CREATED\r\n":"XR_FINGER_ACTIONS_UNAVAILABLE_ANALOG_FALLBACK\r\n");
 if(apply_haptic&&stop_haptic&&ac1_haptic_create(&haptic,action_set,hands.paths,xrCreateAction))out("XR_HAPTIC_ACTION_CREATED\r\n");else out("XR_HAPTIC_OPTIONAL_UNAVAILABLE\r\n");
 unsigned accepted_profiles=0;
 for(unsigned p=0;p<AC1_PROFILE_COUNT;p++){
  if(!profile_enabled[p])continue;
  const Ac1Profile*ps=&ac1_profiles[p];XrPath profile=XR_NULL_PATH;XrResult result=xrStringToPath(inst,ps->path,&profile);
  XrActionSuggestedBinding suggested[64];unsigned n=0;
  if(XR_SUCCEEDED(result))for(unsigned b=0;b<ps->count;b++){
   XrPath path=XR_NULL_PATH;result=xrStringToPath(inst,ps->bindings[b].path,&path);if(XR_FAILED(result))break;
   suggested[n].action=all_actions[ps->bindings[b].action];suggested[n].binding=path;n++;
  }
  if(XR_SUCCEEDED(result)){
   unsigned base_count=n;XrResult pose_result=ac1_hands_append_bindings(&hands,inst,xrStringToPath,suggested,&n,64);
   BOOL pose_ok=XR_SUCCEEDED(pose_result);if(!pose_ok)n=base_count;unsigned poses_count=n;
   XrResult finger_result=ac1_fingers_append(&finger_touch,p,inst,xrStringToPath,suggested,&n,64);
   BOOL fingers_ok=XR_SUCCEEDED(finger_result);if(!fingers_ok)n=poses_count;
   XrInteractionProfileSuggestedBinding suggestion={XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING};suggestion.interactionProfile=profile;suggestion.countSuggestedBindings=n;suggestion.suggestedBindings=suggested;
   unsigned original_count=n;
   BOOL haptic_ok=ac1_haptic_append(&haptic,inst,xrStringToPath,suggested,&n,64);
   if(haptic_ok){suggestion.countSuggestedBindings=n;result=xrSuggestInteractionProfileBindings(inst,&suggestion);haptic_ok=XR_SUCCEEDED(result);}
   if(!haptic_ok){suggestion.countSuggestedBindings=original_count;result=ac1_fingers_suggest(inst,xrSuggestInteractionProfileBindings,&suggestion,base_count,poses_count,&pose_ok,&fingers_ok);}
   if(XR_SUCCEEDED(result)){out("XR_PROFILE_HAPTICS=");out(ps->path);out(haptic_ok?" enabled\r\n":" optional_output_unavailable_controls_preserved\r\n");}
   if(XR_SUCCEEDED(result)&&!pose_ok){out("XR_PROFILE_HANDS_UNAVAILABLE=");out(ps->path);out(" logical_controls_preserved\r\n");}
   if(XR_SUCCEEDED(result)){out("XR_PROFILE_FINGERS=");out(ps->path);out(fingers_ok?" optional_touch_enabled\r\n":" analog_fallback\r\n");}
  }
  out(XR_SUCCEEDED(result)?"XR_PROFILE_ACCEPTED=":"XR_PROFILE_SKIPPED=");out(ps->path);out(" result=");number((DWORD)result);out("\r\n");
  if(XR_SUCCEEDED(result))accepted_profiles++;
  else if(result!=XR_ERROR_PATH_UNSUPPORTED&&result!=XR_ERROR_FEATURE_UNSUPPORTED&&result!=XR_ERROR_EXTENSION_NOT_PRESENT){code=36;goto done;}
 }
 out("XR_PROFILE_COUNT_ACCEPTED=");number(accepted_profiles);out("\r\n");
 if(probe)goto done;
 timer_active=timeBeginPeriod(1)==TIMERR_NOERROR;out("XR_TIMER_PERIOD_MS=1 active=");number(timer_active);out("\r\n");
 DWORD wait_start=GetTickCount();
 while(!mapping&&GetTickCount()-wait_start<20000){mapping=OpenFileMappingW(FILE_MAP_ALL_ACCESS,FALSE,mapname);if(!mapping)Sleep(10);}
 if(!mapping){code=4;goto done;}wire=MapViewOfFile(mapping,FILE_MAP_ALL_ACCESS,0,0,sizeof(*wire));if(!wire||wire->magic!=STEREO_MAGIC||wire->version!=2){code=5;goto done;}
 if(timer_active)InterlockedOr(&wire->timer_flags,2);wire->consumer_pid=GetCurrentProcessId();wait_start=GetTickCount();while(!wire->state&&!wire->stop&&GetTickCount()-wait_start<25000)Sleep(10);
 if(wire->state!=1||wire->stop){code=6;goto done;}
 WCHAR hands_name[160];if(!GetEnvironmentVariableW(L"AC1_ENGINE_HANDS_MAP",hands_name,160)){code=38;goto done;}
 hands_mapping=OpenFileMappingW(FILE_MAP_ALL_ACCESS,FALSE,hands_name);if(!hands_mapping){code=38;goto done;}
 hands_view=MapViewOfFile(hands_mapping,FILE_MAP_ALL_ACCESS,0,0,sizeof(*hands_view));if(!ac1_hands_claim(&hands,hands_view)){code=38;goto done;}
 if(actions_only){wire->luid[0]=req.adapterLuid.LowPart;wire->luid[1]=(DWORD)req.adapterLuid.HighPart;out("INPUT_ACTIONS_ONLY_NO_GAME_IMAGES\r\n");}
 if(FAILED(CreateDXGIFactory(&factory_id,(void**)&factory))){code=7;goto done;}
 for(DWORD i=0;i<12;i++){
  IDXGIAdapter*a=0;if(FAILED(IDXGIFactory_EnumAdapters(factory,i,&a)))break;DXGI_ADAPTER_DESC d;
  if(SUCCEEDED(IDXGIAdapter_GetDesc(a,&d))&&d.AdapterLuid.LowPart==wire->luid[0]&&(DWORD)d.AdapterLuid.HighPart==wire->luid[1]){adapter=a;break;}IDXGIAdapter_Release(a);
 }
 if(!adapter){code=8;goto done;}
 if((req.adapterLuid.LowPart||req.adapterLuid.HighPart)&&(req.adapterLuid.LowPart!=wire->luid[0]||(DWORD)req.adapterLuid.HighPart!=wire->luid[1])){code=24;goto done;}
 D3D_FEATURE_LEVEL level;HRESULT h=D3D11CreateDevice(adapter,D3D_DRIVER_TYPE_UNKNOWN,0,0,0,0,D3D11_SDK_VERSION,&device,&level,&context);
 if(FAILED(h)||level<req.minFeatureLevel){code=9;goto done;}
 for(int i=0;i<2&&!actions_only;i++){
  h=ID3D11Device_OpenSharedResource(device,(HANDLE)(ULONG_PTR)wire->handle[i],&texture_id,(void**)&source[i]);if(FAILED(h)){code=10;goto done;}
  D3D11_TEXTURE2D_DESC d;ID3D11Texture2D_GetDesc(source[i],&d);
  if(d.Width!=wire->width||d.Height!=wire->height||(d.Format!=DXGI_FORMAT_R8G8B8A8_TYPELESS&&d.Format!=DXGI_FORMAT_R8G8B8A8_UNORM)||d.SampleDesc.Count!=1){code=14;goto done;}
 }
 if(!actions_only&&wire->menu_handle&&wire->menu_extension==0x58524d31u){h=ID3D11Device_OpenSharedResource(device,(HANDLE)(ULONG_PTR)wire->menu_handle,&texture_id,(void**)&menu_source);if(FAILED(h)){code=28;goto done;}}
 D3D11_TEXTURE2D_DESC sd={0};sd.Width=wire->width;sd.Height=wire->height;sd.MipLevels=sd.ArraySize=1;sd.Format=DXGI_FORMAT_R8G8B8A8_UNORM;sd.SampleDesc.Count=1;sd.Usage=D3D11_USAGE_STAGING;sd.CPUAccessFlags=D3D11_CPU_ACCESS_READ;
 if(FAILED(ID3D11Device_CreateTexture2D(device,&sd,0,&stage[0]))){code=15;goto done;}
 D3D11_QUERY_DESC qd={D3D11_QUERY_EVENT,0};if(FAILED(ID3D11Device_CreateQuery(device,&qd,&query))){code=16;goto done;}
 XrGraphicsBindingD3D11KHR binding={XR_TYPE_GRAPHICS_BINDING_D3D11_KHR};binding.device=device;
 XrSessionCreateInfo si={XR_TYPE_SESSION_CREATE_INFO};si.next=&binding;si.systemId=system;TRY(xrCreateSession(inst,&si,&session));
 XrReferenceSpaceCreateInfo ri={XR_TYPE_REFERENCE_SPACE_CREATE_INFO};ri.referenceSpaceType=XR_REFERENCE_SPACE_TYPE_LOCAL;ri.poseInReferenceSpace.orientation.w=1;TRY(xrCreateReferenceSpace(session,&ri,&space));
 ri.referenceSpaceType=XR_REFERENCE_SPACE_TYPE_VIEW;TRY(xrCreateReferenceSpace(session,&ri,&view_space));
 TRY(ac1_hands_create_spaces(&hands,session));
 XrSessionActionSetsAttachInfo attach={XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO};attach.countActionSets=1;attach.actionSets=&action_set;TRY(xrAttachSessionActionSets(session,&attach));
 uint32_t image_count[2]={0,0},format_count=0;TRY(xrEnumerateSwapchainFormats(session,0,&format_count,0));
 formats=HeapAlloc(GetProcessHeap(),0,(SIZE_T)format_count*sizeof(*formats));if(!formats){code=37;goto done;}TRY(xrEnumerateSwapchainFormats(session,format_count,&format_count,formats));int64_t chosen_format=0;
 const DXGI_FORMAT preference[]={DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
                                  DXGI_FORMAT_R16G16B16A16_FLOAT,DXGI_FORMAT_R8G8B8A8_UNORM};
 for(unsigned rank=0;rank<sizeof(preference)/sizeof(preference[0])&&!chosen_format;rank++){
  for(uint32_t i=0;i<format_count;i++)if(formats[i]==preference[rank]){chosen_format=formats[i];break;}
 }
 HeapFree(GetProcessHeap(),0,formats);formats=0;
 if(!chosen_format){code=25;goto done;}
 const DXGI_FORMAT display_format=(DXGI_FORMAT)chosen_format;
 if(display_format==DXGI_FORMAT_R16G16B16A16_FLOAT)sd.Format=display_format;
 if(FAILED(ID3D11Device_CreateTexture2D(device,&sd,0,&stage[1]))){code=15;goto done;}
 for(int i=0;i<2;i++){
  XrSwapchainCreateInfo sc={XR_TYPE_SWAPCHAIN_CREATE_INFO};sc.usageFlags=XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT|XR_SWAPCHAIN_USAGE_SAMPLED_BIT|XR_SWAPCHAIN_USAGE_TRANSFER_DST_BIT;sc.format=chosen_format;sc.sampleCount=1;sc.width=wire->width;sc.height=wire->height;sc.faceCount=sc.arraySize=sc.mipCount=1;TRY(xrCreateSwapchain(session,&sc,&swaps[i]));
  TRY(xrEnumerateSwapchainImages(swaps[i],0,&image_count[i],0));if(!image_count[i]){code=17;goto done;}
  images[i]=HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,(SIZE_T)image_count[i]*sizeof(*images[i]));if(!images[i]){code=37;goto done;}
  for(uint32_t j=0;j<image_count[i];j++)images[i][j].type=XR_TYPE_SWAPCHAIN_IMAGE_D3D11_KHR;
  TRY(xrEnumerateSwapchainImages(swaps[i],image_count[i],&image_count[i],(XrSwapchainImageBaseHeader*)images[i]));
 }
 uint32_t menu_count=0;
 if(menu_source){XrSwapchainCreateInfo sc={XR_TYPE_SWAPCHAIN_CREATE_INFO};sc.usageFlags=XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT|XR_SWAPCHAIN_USAGE_SAMPLED_BIT|XR_SWAPCHAIN_USAGE_TRANSFER_DST_BIT;sc.format=chosen_format;sc.sampleCount=1;sc.width=wire->width;sc.height=wire->height;sc.faceCount=sc.arraySize=sc.mipCount=1;TRY(xrCreateSwapchain(session,&sc,&menu_swap));TRY(xrEnumerateSwapchainImages(menu_swap,0,&menu_count,0));if(!menu_count){code=29;goto done;}menu_images=HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,(SIZE_T)menu_count*sizeof(*menu_images));if(!menu_images){code=37;goto done;}for(uint32_t i=0;i<menu_count;i++)menu_images[i].type=XR_TYPE_SWAPCHAIN_IMAGE_D3D11_KHR;TRY(xrEnumerateSwapchainImages(menu_swap,menu_count,&menu_count,(XrSwapchainImageBaseHeader*)menu_images));}
 if(!actions_only){
  h=display_init(&display,device);
  for(int eye=0;eye<2&&SUCCEEDED(h);eye++)
   h=display_chain(&display.eyes[eye],device,source[eye],images[eye],image_count[eye],display_format);
  if(menu_source&&SUCCEEDED(h))
   h=display_chain(&display.menu,device,menu_source,menu_images,menu_count,display_format);
  if(FAILED(h)){out("XR_DISPLAY_INIT_FAILED HRESULT=");number((DWORD)h);out("\r\n");code=50;goto done;}
  h=smaa_init(&smaa,device,wire->width,wire->height);
  smaaReady=SUCCEEDED(h);
  if(!smaaReady){
   out("XR_SMAA_FALLBACK_FXAA HRESULT=");number((DWORD)h);out("\r\n");
  }
  out("XR_DISPLAY_FORMAT=");number((DWORD)display_format);
  out(" NATIVE_FORMAT=");number(wire->format);
  out(smaaReady?" WORLD_SMAA_1X=1 SHADOW_TOE=1\r\n":" WORLD_FXAA=1 SHADOW_TOE=1\r\n");
 }
 out("ACTUAL_NATIVE_STEREO_XR_TAPE_READY\r\n");BOOL running=FALSE,focused=FALSE,tracking_valid=FALSE,profiles_dirty=TRUE;XrTime origin_change_time=0;DWORD cached_tick=0;LONG last_seq=0;BOOL cache_valid=FALSE;XrCompositionLayerProjectionView pv[2]={{XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW},{XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW}};
 XrPath hand_paths[2]={0,0};TRY(xrStringToPath(inst,"/user/hand/left",&hand_paths[0]));TRY(xrStringToPath(inst,"/user/hand/right",&hand_paths[1]));BOOL right_pad_look=FALSE,face_press_before=FALSE;DWORD face_button=0;
 LONG last_menu_seq=0;BOOL menu_valid=FALSE;
 while(!wire->stop&&!wire->error){
  if(wire->drain_requested){ac1_hands_invalidate(&hands);if(query&&!wait_gpu()){code=33;goto done;}InterlockedExchange(&wire->consumer_ack,wire->producer_seq);InterlockedExchange(&wire->menu_ack,wire->menu_seq);wire->pose_valid=0;wire->stop=1;out("XR_GPU_DRAIN_ACKNOWLEDGED\r\n");break;}
  static XrEventDataBuffer ev;memset(&ev,0,sizeof(ev));ev.type=XR_TYPE_EVENT_DATA_BUFFER;
  XrResult poll_result;while((rd_enter("xrPollEvent"),poll_result=xrPollEvent(inst,&ev),rd_leave(poll_result),poll_result)==XR_SUCCESS){if(ev.type==XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED){XrEventDataSessionStateChanged*se=(void*)&ev;focused=se->state==XR_SESSION_STATE_FOCUSED;if(rd)rd->session_state=se->state;if(!focused)ac1_hands_invalidate(&hands);out("XR_SESSION_STATE=");number(se->state);out("\r\n");if(se->state==XR_SESSION_STATE_READY){XrSessionBeginInfo bi={XR_TYPE_SESSION_BEGIN_INFO};bi.primaryViewConfigurationType=XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;TRY(xrBeginSession(session,&bi));running=TRUE;}if(se->state==XR_SESSION_STATE_STOPPING){ac1_haptic_cancel(&haptic,session,stop_haptic);ac1_hands_invalidate(&hands);TRY(xrEndSession(session));running=FALSE;tracking_valid=FALSE;cache_valid=FALSE;InterlockedIncrement(&wire->pose_seq);wire->pose_valid=0;InterlockedIncrement(&wire->pose_seq);InterlockedIncrement(&wire->input_seq);wire->input_valid=0;InterlockedIncrement(&wire->input_seq);out("XR_SESSION_PAUSED_KEEP_NATIVE_GAME\r\n");}if(se->state==XR_SESSION_STATE_EXITING||se->state==XR_SESSION_STATE_LOSS_PENDING)goto done;}else if(ev.type==XR_TYPE_EVENT_DATA_INTERACTION_PROFILE_CHANGED){profiles_dirty=TRUE;}else if(ev.type==XR_TYPE_EVENT_DATA_REFERENCE_SPACE_CHANGE_PENDING){XrEventDataReferenceSpaceChangePending*change=(void*)&ev;if(change->referenceSpaceType==XR_REFERENCE_SPACE_TYPE_LOCAL)origin_change_time=change->changeTime;}ev.type=XR_TYPE_EVENT_DATA_BUFFER;}
  if(!focused)ac1_haptic_cancel(&haptic,session,stop_haptic);
  if(!running){Sleep(5);continue;}
  XrFrameWaitInfo fw={XR_TYPE_FRAME_WAIT_INFO};XrFrameState fs={XR_TYPE_FRAME_STATE};TRY(xrWaitFrame(session,&fw,&fs));XrFrameBeginInfo fb={XR_TYPE_FRAME_BEGIN_INFO};TRY(xrBeginFrame(session,&fb));if(rd){rd->frame++;rd->should_render=fs.shouldRender;}
  XrActiveActionSet active={action_set,XR_NULL_PATH};XrActionsSyncInfo sync={XR_TYPE_ACTIONS_SYNC_INFO};sync.countActiveActionSets=1;sync.activeActionSets=&active;TRY(xrSyncActions(session,&sync));
  if(profiles_dirty){
   for(unsigned hand=0;hand<2;hand++){
    XrInteractionProfileState state={XR_TYPE_INTERACTION_PROFILE_STATE};TRY(xrGetCurrentInteractionProfile(session,hand_paths[hand],&state));
    char path[XR_MAX_PATH_LENGTH]={0};uint32_t length=0;
    if(state.interactionProfile)TRY(xrPathToString(inst,state.interactionProfile,sizeof(path),&length,path));
    out("XR_CURRENT_PROFILE hand=");out(hand?"right":"left");out(" profile=");out(state.interactionProfile?path:"UNBOUND");out("\r\n");
    if(hand)right_pad_look=state.interactionProfile&&!lstrcmpA(path,"/interaction_profiles/htc/vive_controller");
   }
   profiles_dirty=FALSE;
  }
  XrActionStateGetInfo get={XR_TYPE_ACTION_STATE_GET_INFO};get.action=confirm_action;XrActionStateBoolean confirm={XR_TYPE_ACTION_STATE_BOOLEAN};TRY(xrGetActionStateBoolean(session,&get,&confirm));
  DWORD mask=confirm.isActive&&confirm.currentState?1u:0u,active_mask=confirm.isActive?1u:0u;static const DWORD bit[6]={3,1,2,9,14,15};float values[4]={0};XrVector2f sticks[2]={{0}};
  for(int i=0;i<12;i++){get.action=controls[i];if(i<6){XrActionStateBoolean v={XR_TYPE_ACTION_STATE_BOOLEAN};TRY(xrGetActionStateBoolean(session,&get,&v));if(v.isActive)active_mask|=1u<<(i+1);if(v.isActive&&v.currentState)mask|=1u<<bit[i];}else if(i<10){XrActionStateFloat v={XR_TYPE_ACTION_STATE_FLOAT};TRY(xrGetActionStateFloat(session,&get,&v));if(v.isActive){active_mask|=1u<<(i+1);values[i-6]=v.currentState;}}else{XrActionStateVector2f v={XR_TYPE_ACTION_STATE_VECTOR2F};TRY(xrGetActionStateVector2f(session,&get,&v));if(v.isActive){active_mask|=1u<<(i+1);sticks[i-10]=v.currentState;}}}
  if(values[0]>.12f)mask|=1u<<10;if(values[1]>.12f)mask|=1u<<12;if(values[2]>.6f)mask|=1u<<11;if(values[3]>.6f)mask|=1u<<13;
  get.action=all_actions[13];XrActionStateVector2f face_pad={XR_TYPE_ACTION_STATE_VECTOR2F};TRY(xrGetActionStateVector2f(session,&get,&face_pad));if(face_pad.isActive)active_mask|=1u<<13;
  get.action=all_actions[14];XrActionStateBoolean face_click={XR_TYPE_ACTION_STATE_BOOLEAN};TRY(xrGetActionStateBoolean(session,&get,&face_click));if(face_click.isActive)active_mask|=1u<<14;
  BOOL face_down=face_pad.isActive&&face_click.isActive&&face_click.currentState&&(!real||focused);
  if(face_down&&!face_press_before){float x=face_pad.currentState.x,y=face_pad.currentState.y;float ax=x<0?-x:x,ay=y<0?-y:y;face_button=ax<.25f&&ay<.25f?1u:ax>ay?(x<0?1u<<3:1u):(y<0?1u<<1:1u<<2);}
  if(face_down)mask|=face_button;else face_button=0;face_press_before=face_down;
  for(unsigned i=0;i<2;i++){get.action=all_actions[15+i];XrActionStateBoolean touch={XR_TYPE_ACTION_STATE_BOOLEAN};TRY(xrGetActionStateBoolean(session,&get,&touch));if(touch.isActive){active_mask|=1u<<(15+i);if(!touch.currentState)sticks[i].x=sticks[i].y=0;}}
  if(right_pad_look&&face_down)sticks[1].x=sticks[1].y=0;
  DWORD logical_confirm=!!(mask&1u);
  InterlockedIncrement(&wire->input_seq);if(wire->confirm_a!=logical_confirm){wire->input_transitions++;out("XR_CONFIRM_A=");number(logical_confirm);out(" active=");number(active_mask!=0);out("\r\n");}
  wire->input_valid=active_mask!=0&&(!real||focused);wire->confirm_a=logical_confirm;wire->buttons=mask;memcpy(wire->stick_l,&sticks[0],8);memcpy(wire->stick_r,&sticks[1],8);wire->trigger_l=values[0];wire->trigger_r=values[1];wire->grip_l=values[2];wire->grip_r=values[3];wire->input_active=active_mask;wire->input_extension=0x58524931u;wire->input_tick=GetTickCount();memcpy(wire->input_time,&fs.predictedDisplayTime,8);InterlockedIncrement(&wire->input_seq);
  XrViewLocateInfo vl={XR_TYPE_VIEW_LOCATE_INFO};vl.viewConfigurationType=XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;vl.displayTime=fs.predictedDisplayTime;vl.space=space;XrViewState vs={XR_TYPE_VIEW_STATE};XrView views[2]={{XR_TYPE_VIEW},{XR_TYPE_VIEW}};uint32_t count=0;TRY(xrLocateViews(session,&vl,&vs,2,&count,views));
  XrViewStateFlags required=XR_VIEW_STATE_ORIENTATION_VALID_BIT|XR_VIEW_STATE_POSITION_VALID_BIT;if(real)required|=XR_VIEW_STATE_ORIENTATION_TRACKED_BIT|XR_VIEW_STATE_POSITION_TRACKED_BIT;
  BOOL origin_changed=origin_change_time&&fs.predictedDisplayTime>=origin_change_time;
  BOOL located=count==2&&(vs.viewStateFlags&required)==required&&(!real||fs.shouldRender);
  if(origin_changed){origin_change_time=0;cache_valid=FALSE;out("XR_LOCAL_ORIGIN_CHANGED\r\n");}
  if(located){
   InterlockedIncrement(&wire->pose_seq);wire->pose_valid=1;wire->pose_tick=GetTickCount();
   if(!tracking_valid||origin_changed){wire->pose_generation++;out("XR_TRACKING_RESUMED generation=");number(wire->pose_generation);out("\r\n");}tracking_valid=TRUE;
   for(int e=0;e<2;e++){memcpy(wire->eye[e],&views[e].pose.orientation,16);memcpy(wire->eye[e]+4,&views[e].pose.position,12);memcpy(wire->eye[e]+7,&views[e].fov,16);}memcpy(wire->pose_time,&fs.predictedDisplayTime,8);InterlockedIncrement(&wire->pose_seq);
  }else{InterlockedIncrement(&wire->pose_seq);wire->pose_valid=0;InterlockedIncrement(&wire->pose_seq);if(tracking_valid)out("XR_TRACKING_PAUSED\r\n");tracking_valid=FALSE;cache_valid=FALSE;}
  {
   DWORD touch_buttons=0,touch_active=0;
   ac1_fingers_sample(&finger_touch,session,running&&focused&&fs.shouldRender&&located,xrGetActionStateBoolean,&touch_buttons,&touch_active);
   DWORD hand_mask=ac1_hands_sample(&hands,session,space,fs.predictedDisplayTime,running,focused,fs.shouldRender,located,wire->pose_generation,(DWORD)wire->pose_seq,views,mask|touch_buttons,active_mask|touch_active,values,sticks);
   static DWORD previous_touch=0xffffffffu;if(touch_active!=previous_touch){out("XR_FINGER_TOUCH_ACTIVE_MASK=");number(touch_active>>24);out("\r\n");previous_touch=touch_active;}
   static DWORD previous_hand_mask=0xffffffffu;if(hand_mask!=previous_hand_mask){out("XR_HANDS_USABLE_MASK=");number(hand_mask);out(" generation=");number(wire->pose_generation);out("\r\n");previous_hand_mask=hand_mask;}
  }
  if(ac1_haptic_service(&haptic,wire,session,running&&focused&&fs.shouldRender&&located,GetTickCount(),apply_haptic,stop_haptic)){
   out("XR_HAPTIC_EVENT seq=");number((DWORD)haptic.sequence);out(" applied=");number(haptic.applied);out(" dropped=");number(haptic.dropped);out(" failed=");number(haptic.failed);out(" result=");signed_number(haptic.result);out("\r\n");
  }
  LONG pair_seq=wire->producer_seq;BOOL fresh=pair_seq!=last_seq;
  if(fresh&&real&&(!located||wire->pair_pose_generation!=wire->pose_generation)){last_seq=pair_seq;cache_valid=FALSE;InterlockedExchange(&wire->consumer_ack,pair_seq);fresh=FALSE;}
  /* A delayed producer/transition is not a fatal runtime error. Do not copy
     stale pixels; release this completed publication and await a new pose. */
  if(fresh&&GetTickCount()-wire->pair_pose_tick>max_pair_age){out("XR_STALE_PAIR_DROPPED seq=");number((DWORD)pair_seq);out(" age_ms=");number(GetTickCount()-wire->pair_pose_tick);out("\r\n");last_seq=pair_seq;/* Keep the previous completed same-generation image. */InterlockedExchange(&wire->consumer_ack,pair_seq);fresh=FALSE;}
  if(fresh){
   DWORD hashes[2]={0,0};
   for(int e=0;e<2;e++){
    uint32_t index=0;XrSwapchainImageAcquireInfo ac={XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO};TRY(xrAcquireSwapchainImage(swaps[e],&ac,&index));if(index>=image_count[e]){code=19;goto done;}
    XrSwapchainImageWaitInfo wi={XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO};wi.timeout=1000000000;TRY(xrWaitSwapchainImage(swaps[e],&wi));
    if((!real&&!sim_present)||wire->verified_eyes<4){
     display_draw(&display,&display.eyes[e],context,index,wire->width,wire->height,FALSE);
     if(!verify_transfer(source[e],images[e][index].texture,display_format,&hashes[e])){
      out("XR_EYE_COLOR_TRANSFER_VERIFICATION_FAILED\r\n");code=20;goto done;
     }
     wire->verified_eyes++;
    }
    if(smaaReady)smaa_draw(&smaa,&display,&display.eyes[e],context,index,wire->width,wire->height);
    else display_draw(&display,&display.eyes[e],context,index,wire->width,wire->height,TRUE);
    memcpy(&pv[e].pose.orientation,wire->pair_eye[e],16);memcpy(&pv[e].pose.position,wire->pair_eye[e]+4,12);memcpy(&pv[e].fov,wire->pair_eye[e]+7,16);
    pv[e].subImage.swapchain=swaps[e];pv[e].subImage.imageRect.extent.width=wire->width;pv[e].subImage.imageRect.extent.height=wire->height;
   }
   /* Both finished eye draws must complete before release and producer ACK. */
   if(!wait_gpu()){code=26;goto done;}
   for(int e=0;e<2;e++){XrSwapchainImageReleaseInfo re={XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO};TRY(xrReleaseSwapchainImage(swaps[e],&re));}
   cache_valid=TRUE;cached_tick=GetTickCount();last_seq=pair_seq;
   LARGE_INTEGER ql,qr,qf;memcpy(&ql,wire->native_qpc,8);memcpy(&qr,wire->native_qpc+2,8);memcpy(&qf,wire->qpc_frequency,8);
   out("PAIR seq=");number((DWORD)pair_seq);out(" left_frame=");number(wire->native_frame[0]);out(" right_frame=");number(wire->native_frame[1]);out(" delta_us=");number((DWORD)((double)(qr.QuadPart-ql.QuadPart)*1000000.0/(double)qf.QuadPart));out(" hashL=");number(hashes[0]);out(" hashR=");number(hashes[1]);out(" verified_eyes=");number(wire->verified_eyes);out("\r\n");
   /* The shared textures are no longer read after the completed fence. Pair
      poses and metadata above are now local. Let the producer render the next
      pair without waiting for this independent xrEndFrame call. */
   InterlockedExchange(&wire->consumer_ack,pair_seq);
  }
  LONG menu_seq=wire->menu_seq;BOOL fresh_menu=menu_source&&menu_seq!=last_menu_seq;
  if(fresh_menu){
   uint32_t index=0;
   XrSwapchainImageAcquireInfo ac={XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO};
   TRY(xrAcquireSwapchainImage(menu_swap,&ac,&index));
   if(index>=menu_count){code=30;goto done;}
   XrSwapchainImageWaitInfo wi={XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO};wi.timeout=1000000000;
   TRY(xrWaitSwapchainImage(menu_swap,&wi));
   display_draw(&display,&display.menu,context,index,wire->width,wire->height,FALSE);
   DWORD hash=0;
   if((!real&&!sim_present)||menu_seq<=2){
    if(!verify_transfer(menu_source,menu_images[index].texture,display_format,&hash)){
     out("XR_MENU_COLOR_TRANSFER_VERIFICATION_FAILED\r\n");code=31;goto done;
    }
    wire->menu_verified++;
   }else if(!wait_gpu()){code=32;goto done;}
   XrSwapchainImageReleaseInfo re={XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO};
   TRY(xrReleaseSwapchainImage(menu_swap,&re));
   last_menu_seq=menu_seq;menu_valid=TRUE;
   out("UI_COPY seq=");number(menu_seq);out(" native_frame=");number(wire->menu_frame);
   out(" hash=");number(hash);out(" verified=");number(wire->menu_verified);out("\r\n");
  }
  XrCompositionLayerProjection layer={XR_TYPE_COMPOSITION_LAYER_PROJECTION};layer.space=space;layer.viewCount=2;layer.views=pv;
  /* Only a qualified current camera clears menu_visible. Native 2D-only UI
     fills both eye fields instead of becoming a floating cinema rectangle.
     This fallback has no invented depth; every 3D camera above is rerendered
     per eye. Its native UI copy and GPU lifetime keep the existing ACK ABI. */
  BOOL world_ready=(!real||(located&&last_seq>2))&&cache_valid;
   /* Re-submit the last GPU-complete eye images and their ORIGINAL render
      poses. An ordinary producer delay is not a tracking/reference change.
      Lost tracking, origin/session changes and drain still invalidate above. */
   static BOOL reporting_hold=FALSE;
   DWORD image_age=GetTickCount()-cached_tick;
   if(world_ready&&!fresh&&image_age>=200){if(!reporting_hold){out("XR_COMPLETED_PAIR_RETAINED seq=");number((DWORD)last_seq);out(" age_ms=");number(image_age);out("\r\n");reporting_hold=TRUE;}}
   else if(reporting_hold){out("XR_COMPLETED_PAIR_RETENTION_ENDED seq=");number((DWORD)last_seq);out("\r\n");reporting_hold=FALSE;}
  XrCompositionLayerProjectionView ui_views[2]={{XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW},{XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW}};
  XrCompositionLayerProjection ui={XR_TYPE_COMPOSITION_LAYER_PROJECTION};ui.space=space;ui.viewCount=2;ui.views=ui_views;
  const XrCompositionLayerBaseHeader*layers[1];uint32_t layer_count=0,presentation=0;
  if((!real||fs.shouldRender)&&count==2&&(vs.viewStateFlags&(XR_VIEW_STATE_ORIENTATION_VALID_BIT|XR_VIEW_STATE_POSITION_VALID_BIT))==(XR_VIEW_STATE_ORIENTATION_VALID_BIT|XR_VIEW_STATE_POSITION_VALID_BIT)&&menu_valid&&wire->menu_visible){
   XrPosef ui_pose;XrFovf ui_fov;ac1_ui_projection(views,wire->width,wire->height,&ui_pose,&ui_fov);
   for(int e=0;e<2;e++){ui_views[e].pose=ui_pose;ui_views[e].fov=ui_fov;ui_views[e].subImage.swapchain=menu_swap;ui_views[e].subImage.imageRect.extent.width=wire->width;ui_views[e].subImage.imageRect.extent.height=wire->height;}
   layers[layer_count++]=(const XrCompositionLayerBaseHeader*)&ui;presentation=2;
  }else if(world_ready){layers[layer_count++]=(const XrCompositionLayerBaseHeader*)&layer;wire->consumer_frames++;presentation=1;}
  static DWORD previous_presentation=0xffffffffu;if(presentation!=previous_presentation){out("XR_PRESENTATION=");out(presentation==1?"NATIVE_STEREO":presentation==2?"NATIVE_2D_UI_PROJECTION":"NO_RENDERABLE_FRAME");out(" seq=");number((DWORD)last_seq);out("\r\n");previous_presentation=presentation;}
  XrFrameEndInfo fe={XR_TYPE_FRAME_END_INFO};fe.displayTime=fs.predictedDisplayTime;fe.environmentBlendMode=XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
  fe.layerCount=layer_count;fe.layers=layer_count?layers:0;
  TRY(xrEndFrame(session,&fe));if(fresh_menu)InterlockedExchange(&wire->menu_ack,menu_seq);if((!real&&!sim_present)||!fs.shouldRender)Sleep(5);
 }
done:
 ac1_haptic_cancel(&haptic,session,stop_haptic);
 /* Loss/exit paths must finish our context's submitted copies too. */
 BOOL final_drained=!context||!query||wait_gpu();
 if(!final_drained){out("XR_FINAL_GPU_DRAIN_FAILED\r\n");if(!code)code=34;}
 if(wire){wire->pose_valid=0;wire->input_valid=0;wire->stop=1;if(code)wire->error=code;if(final_drained){InterlockedExchange(&wire->consumer_ack,wire->producer_seq);InterlockedExchange(&wire->menu_ack,wire->menu_seq);out("XR_GPU_DRAIN_ACKNOWLEDGED\r\n");}}
 ac1_hands_close(&hands,destroy_space);
 smaa_close(&smaa);
 display_close(&display);
 for(int i=0;i<2;i++)if(swaps[i]&&destroy_swap)destroy_swap(swaps[i]);if(menu_swap&&destroy_swap)destroy_swap(menu_swap);if(view_space&&destroy_space)destroy_space(view_space);if(space&&destroy_space)destroy_space(space);if(session&&destroy_session)destroy_session(session);if(action_set&&destroy_action_set)destroy_action_set(action_set);if(inst&&destroy_instance)destroy_instance(inst);
 if(query)ID3D11Query_Release(query);if(menu_source)ID3D11Texture2D_Release(menu_source);for(int i=0;i<2;i++){if(stage[i])ID3D11Texture2D_Release(stage[i]);if(source[i])ID3D11Texture2D_Release(source[i]);}if(context)ID3D11DeviceContext_Release(context);if(device)ID3D11Device_Release(device);if(adapter)IDXGIAdapter_Release(adapter);if(factory)IDXGIFactory_Release(factory);
 if(available_extensions)HeapFree(GetProcessHeap(),0,available_extensions);if(formats)HeapFree(GetProcessHeap(),0,formats);for(unsigned i=0;i<2;i++)if(images[i])HeapFree(GetProcessHeap(),0,images[i]);if(menu_images)HeapFree(GetProcessHeap(),0,menu_images);
 if(hands_view)UnmapViewOfFile(hands_view);if(hands_mapping)CloseHandle(hands_mapping);
 if(timer_active){timeEndPeriod(1);if(wire)InterlockedAnd(&wire->timer_flags,~2L);}if(wire)UnmapViewOfFile(wire);if(mapping)CloseHandle(mapping);if(tape)FreeLibrary(tape);if(runtime)FreeLibrary(runtime);
 out("XR_STEREO_EXIT=");number(code);out("\r\n");rd_exit(code);ExitProcess(code);return code;
}
