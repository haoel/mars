//
// Created by Changpeng Pan on 2022/9/14.
//

#include "stn_manager_callback_wrapper.h"

#include "comm/jni/jnicat/jnicat_core.h"
#include "mars/comm/jni/util/comm_function.h"
#include "mars/comm/jni/util/scope_jenv.h"
#include "mars/comm/jni/util/scoped_jstring.h"
#include "mars/comm/jni/util/var_cache.h"
#include "mars/comm/thread/lock.h"
#include "mars/comm/thread/mutex.h"

namespace mars {
namespace stn {

StnManagerJniCallback::StnManagerJniCallback(JNIEnv* env, jobject callback) {
    callback_inst_ = env->NewGlobalRef(callback);
    jclass objClass = env->GetObjectClass(callback);
    if (objClass) {
        callbacks_class_ = reinterpret_cast<jclass>(env->NewGlobalRef(objClass));
        env->DeleteLocalRef(objClass);
    }
}

StnManagerJniCallback::~StnManagerJniCallback() {
    jnienv_ptr env;
    env->DeleteGlobalRef(callback_inst_);
}

JNICAT_DEFINE_METHOD(kStnManagerJniCallback_MakesureAuthed, "com/tencent/mars/stn/StnManager$CallBack", "makesureAuthed", "(Ljava/lang/String;Ljava/lang/String;)Z")
bool StnManagerJniCallback::MakesureAuthed(const std::string& _host, const std::string& _user_id) {
    jnienv_ptr env;
    return j2c_cast(c2j_call(jboolean, callback_inst_, kStnManagerJniCallback_MakesureAuthed, c2j_cast(jstring, _user_id)));
}

JNICAT_DEFINE_METHOD(kStnManagerJniCallback_TrafficData, "com/tencent/mars/stn/StnManager$CallBack", "trafficData", "(II)V")
void StnManagerJniCallback::TrafficData(ssize_t _send, ssize_t _recv) {
    jnienv_ptr env;
    c2j_call(void, callback_inst_, kStnManagerJniCallback_TrafficData, c2j_cast(jint, _send), c2j_cast(jint, _recv));
}

JNICAT_DEFINE_METHOD(kStnManagerJniCallback_OnNewDns, "com/tencent/mars/stn/StnManager$CallBack", "onNewDns", "(Ljava/lang/String;)[Ljava/lang/String;")
std::vector<std::string> StnManagerJniCallback::OnNewDns(const std::string& host, bool longlink_host) {
    jnienv_ptr env;
    std::vector<std::string> iplist;
    if (!host.empty()) {
        jobjectArray ip_strs = c2j_call(jobjectArray, callback_inst_, kStnManagerJniCallback_OnNewDns, c2j_cast(jstring, host), c2j_cast(jboolean, longlink_host));
        if (ip_strs != NULL) {
            jsize size = env->GetArrayLength(ip_strs);
            for (int i = 0; i < size; i++) {
                jstring ip = (jstring)env->GetObjectArrayElement(ip_strs, i);
                if (ip != NULL) {
                    iplist.push_back(ScopedJstring(env, ip).GetChar());
                }
                JNU_FreeJstring(env, ip);
            }
            env->DeleteLocalRef(ip_strs);
        }
    } else {
        // xerror2(TSF"host is empty");
    }

    return iplist;
}

JNICAT_DEFINE_METHOD(kStnManagerJniCallback_OnPush, "com/tencent/mars/stn/StnManager$CallBack", "onPush", "(Ljava/lang/String;II[B[B)V")
void StnManagerJniCallback::OnPush(const std::string& _channel_id, uint32_t _cmdid, uint32_t _taskid, const AutoBuffer& _body, const AutoBuffer& _extend) {
    jnienv_ptr env;
    jbyteArray data_jba = NULL;
    if (_body.Length() > 0) {
        data_jba = JNU_Buffer2JbyteArray(env, _body);
    } else {
        // xdebug2(TSF"the data.Lenght() < = 0");
    }
    c2j_call(void, callback_inst_, kStnManagerJniCallback_OnPush, c2j_cast(jstring, _channel_id), c2j_cast(jint, _cmdid), c2j_cast(jint, _taskid), c2j_cast(jbyteArray, _body), c2j_cast(jbyteArray, _extend));
}

JNICAT_DEFINE_METHOD(kStnManagerJniCallback_Req2Buf, "com/tencent/mars/stn/StnManager$CallBack", "req2Buf", "(ILjava/lang/Object;Ljava/lang/String;Ljava/io/ByteArrayOutputStream;[B[IILjava/lang/String;)Z")
bool StnManagerJniCallback::Req2Buf(uint32_t _taskid, void* const _user_context, const std::string& _user_id, AutoBuffer& outbuffer, AutoBuffer& extend, int& error_code, const int channel_select, const std::string& host) {
    jnienv_ptr env;
    return j2c_cast(c2j_call(jboolean,
                             callback_inst_,
                             kStnManagerJniCallback_Req2Buf,
                             c2j_cast(jint, _taskid),
                             _user_context,
                             c2j_cast(jstring, _user_id),
                             c2j_cast(jbyteArray, outbuffer),
                             c2j_cast(jbyteArray, extend),
                             c2j_cast(jint, error_code),
                             c2j_cast(jint, channel_select),
                             c2j_cast(jstring, host)));
}

JNICAT_DEFINE_METHOD(kStnManagerJniCallback_Buf2Resp, "com/tencent/mars/stn/StnManager$CallBack", "buf2Resp", "(ILjava/lang/Object;Ljava/lang/String;[B[B[II)I")
int StnManagerJniCallback::Buf2Resp(uint32_t _taskid, void* const _user_context, const std::string& _user_id, const AutoBuffer& _inbuffer, const AutoBuffer& _extend, int& _error_code, const int _channel_select) {
    jnienv_ptr env;
    return j2c_cast(c2j_call(jint,
                             callback_inst_,
                             kStnManagerJniCallback_Buf2Resp,
                             c2j_cast(jint, _taskid),
                             _user_context,
                             c2j_cast(jstring, _user_id),
                             c2j_cast(jbyteArray, _inbuffer),
                             c2j_cast(jbyteArray, _extend),
                             c2j_cast(jint, _error_code),
                             c2j_cast(jint, _channel_select)));
}

DEFINE_FIND_CLASS(KC2JavaStnCgiProfile, "com/tencent/mars/stn/StnLogic$CgiProfile");
JNICAT_DEFINE_METHOD(kStnManagerJniCallback_OnTaskEnd, "com/tencent/mars/stn/StnManager$CallBack", "onTaskEnd", "(ILjava/lang/Object;Ljava/lang/String;IILcom/tencent/mars/stn/StnLogic$CgiProfile;)I")
int StnManagerJniCallback::OnTaskEnd(uint32_t _taskid, void* const _user_context, const std::string& _user_id, int _error_type, int _error_code, const CgiProfile& _profile) {
    jnienv_ptr env;
    VarCache* cache_instance = VarCache::Singleton();

    jclass cgiProfileCls = cache_instance->GetClass(env, KC2JavaStnCgiProfile);
    jmethodID jobj_init = env->GetMethodID(cgiProfileCls, "<init>", "()V");
    jobject jobj_cgiItem = env->NewObject(cgiProfileCls, jobj_init);
    if (nullptr == jobj_cgiItem) {
        env->ThrowNew(env->FindClass("java/lang/RuntimeException"), "C2Java_OnTaskEnd: create jobject failed.");
        return -1;
    }

    jfieldID fid_taskStartTime = env->GetFieldID(cgiProfileCls, "taskStartTime", "J");
    jfieldID fid_startConnectTime = env->GetFieldID(cgiProfileCls, "startConnectTime", "J");
    jfieldID fid_connectSuccessfulTime = env->GetFieldID(cgiProfileCls, "connectSuccessfulTime", "J");
    jfieldID fid_startHandshakeTime = env->GetFieldID(cgiProfileCls, "startHandshakeTime", "J");
    jfieldID fid_handshakeSuccessfulTime = env->GetFieldID(cgiProfileCls, "handshakeSuccessfulTime", "J");
    jfieldID fid_startSendPacketTime = env->GetFieldID(cgiProfileCls, "startSendPacketTime", "J");
    jfieldID fid_startReadPacketTime = env->GetFieldID(cgiProfileCls, "startReadPacketTime", "J");
    jfieldID fid_readPacketFinishedTime = env->GetFieldID(cgiProfileCls, "readPacketFinishedTime", "J");
    jfieldID fid_rtt = env->GetFieldID(cgiProfileCls, "rtt", "J");
    jfieldID fid_channelType = env->GetFieldID(cgiProfileCls, "channelType", "I");

    return j2c_cast(c2j_call(jint, callback_inst_, kStnManagerJniCallback_OnTaskEnd, c2j_cast(jint, _taskid), _user_context, c2j_cast(jstring, _user_id), c2j_cast(jint, _error_type), c2j_cast(jint, _error_code), cgiProfileCls));
}

JNICAT_DEFINE_METHOD(kStnManagerJniCallback_ReportConnectStatus, "com/tencent/mars/stn/StnManager$CallBack", "reportConnectStatus", "(II)V")
void StnManagerJniCallback::ReportConnectStatus(int _status, int _longlink_status) {
    jnienv_ptr env;
    c2j_call(void, callback_inst_, kStnManagerJniCallback_ReportConnectStatus, c2j_cast(jint, _status), c2j_cast(jint, _longlink_status));
}

JNICAT_DEFINE_CLASS("com/tencent/mars/stn/Stn$ErrCmdType")
JNICAT_DEFINE_METHOD(kStnManagerJniCallback_OnLongLinkNetworkError, "com/tencent/mars/stn/StnManager$CallBack", "onLongLinkNetworkError", "(Lcom/tencent/mars/stn/Stn$ErrCmdType;ILjava/lang/String;I)V")
void StnManagerJniCallback::OnLongLinkNetworkError(ErrCmdType _err_type, int _err_code, const std::string& _ip, uint16_t _port) {
    jnienv_ptr env;
    c2j_call(void, callback_inst_, kStnManagerJniCallback_OnLongLinkNetworkError, c2j_enum_cast(_err_type, "com/tencent/mars/stn/Stn$ErrCmdType"), c2j_cast(jint, _err_code), c2j_cast(jstring, _ip), c2j_cast(jint, _port));
}

JNICAT_DEFINE_METHOD(kStnManagerJniCallback_OnShortLinkNetworkError, "com/tencent/mars/stn/StnManager$CallBack", "onShortLinkNetworkError", "(Lcom/tencent/mars/stn/Stn$ErrCmdType;ILjava/lang/String;Ljava/lang/String;I)V")
void StnManagerJniCallback::OnShortLinkNetworkError(ErrCmdType _err_type, int _err_code, const std::string& _ip, const std::string& _host, uint16_t _port) {
    jnienv_ptr env;
    c2j_call(void,
             callback_inst_,
             kStnManagerJniCallback_OnShortLinkNetworkError,
             c2j_enum_cast(_err_type, "com/tencent/mars/stn/Stn$ErrCmdType"),
             c2j_cast(jint, _err_code),
             c2j_cast(jstring, _ip),
             c2j_cast(jstring, _host),
             c2j_cast(jint, _port));
}

JNICAT_DEFINE_METHOD(kStnManagerJniCallback_OnLongLinkStatusChange, "com/tencent/mars/stn/StnManager$CallBack", "onLongLinkStatusChange", "(I)V")
void StnManagerJniCallback::OnLongLinkStatusChange(int _status) {
    jnienv_ptr env;
    c2j_call(void, callback_inst_, kStnManagerJniCallback_OnLongLinkStatusChange, c2j_cast(jint, _status));
}

JNICAT_DEFINE_METHOD(kStnManagerJniCallback_GetLonglinkIdentifyCheckBuffer,
                     "com/tencent/mars/stn/StnManager$CallBack",
                     "getLonglinkIdentifyCheckBuffer",
                     "(Ljava/lang/String;Ljava/io/ByteArrayOutputStream;Ljava/io/ByteArrayOutputStream;[I)I")
int StnManagerJniCallback::GetLonglinkIdentifyCheckBuffer(const std::string& _channel_id, AutoBuffer& _identify_buffer, AutoBuffer& _buffer_hash, int32_t& _cmdid) {
    jnienv_ptr env;
    return j2c_cast(c2j_call(jint, callback_inst_, kStnManagerJniCallback_GetLonglinkIdentifyCheckBuffer, c2j_cast(jstring, _channel_id), c2j_cast(jbyteArray, _identify_buffer), c2j_cast(jbyteArray, _buffer_hash), c2j_cast(jint, _cmdid)));
}

JNICAT_DEFINE_METHOD(kStnManagerJniCallback_OnLonglinkIdentifyResponse, "com/tencent/mars/stn/StnManager$CallBack", "onLonglinkIdentifyResponse", "(Ljava/lang/String;[B[B)Z")
bool StnManagerJniCallback::OnLonglinkIdentifyResponse(const std::string& _channel_id, const AutoBuffer& _response_buffer, const AutoBuffer& _identify_buffer_hash) {
    jnienv_ptr env;
    return j2c_cast(c2j_call(jboolean, callback_inst_, kStnManagerJniCallback_OnLonglinkIdentifyResponse, c2j_cast(jstring, _channel_id), c2j_cast(jbyteArray, _response_buffer), c2j_cast(jbyteArray, _identify_buffer_hash)));
}

JNICAT_DEFINE_METHOD(kStnManagerJniCallback_RequestSync, "com/tencent/mars/stn/StnManager$CallBack", "requestSync", "()V")
void StnManagerJniCallback::RequestSync() {
    jnienv_ptr env;
    c2j_call(void, callback_inst_, kStnManagerJniCallback_RequestSync);
}

}  // namespace stn
}  // namespace mars