#include <arpa/inet.h>
#include <netdb.h>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include "udt.h"
#include <jni.h>
#include <string.h>
#include <android/log.h>
using namespace std;

#define TAG "myDemo-jni" // 这个是自定义的LOG的标识
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,TAG ,__VA_ARGS__) // 定义LOGD类型
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,TAG ,__VA_ARGS__) // 定义LOGI类型
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,TAG ,__VA_ARGS__) // 定义LOGW类型
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG ,__VA_ARGS__) // 定义LOGE类型
#define LOGF(...) __android_log_print(ANDROID_LOG_FATAL,TAG ,__VA_ARGS__) // 定义LOGF类型
extern "C" {
jstring charTojstring(JNIEnv* env, const char* pat);
char* jstringToChar(JNIEnv* env, jstring jstr);
JNIEXPORT jint JNICALL Java_net_meoi_udtrecv_MainActivity_RecvFileFromServer
  (JNIEnv *, jclass, jstring, jstring, jstring, jstring);
}

jstring charTojstring(JNIEnv* env, const char* pat) {
    //定义java String类 strClass
    jclass strClass = (env)->FindClass("Ljava/lang/String;");
    //获取String(byte[],String)的构造器,用于将本地byte[]数组转换为一个新String
    jmethodID ctorID = (env)->GetMethodID(strClass, "<init>", "([BLjava/lang/String;)V");
    //建立byte数组
    jbyteArray bytes = (env)->NewByteArray(strlen(pat));
    //将char* 转换为byte数组
    (env)->SetByteArrayRegion(bytes, 0, strlen(pat), (jbyte*) pat);
    // 设置String, 保存语言类型,用于byte数组转换至String时的参数
    jstring encoding = (env)->NewStringUTF("GB2312");
    //将byte数组转换为java String,并输出
    return (jstring) (env)->NewObject(strClass, ctorID, bytes, encoding);
}

char* jstringToChar(JNIEnv* env, jstring jstr) {
    char* rtn = NULL;
    jclass clsstring = env->FindClass("java/lang/String");
    jstring strencode = env->NewStringUTF("GB2312");
    jmethodID mid = env->GetMethodID(clsstring, "getBytes", "(Ljava/lang/String;)[B");
    jbyteArray barr = (jbyteArray) env->CallObjectMethod(jstr, mid, strencode);
    jsize alen = env->GetArrayLength(barr);
    jbyte* ba = env->GetByteArrayElements(barr, JNI_FALSE);
    if (alen > 0) {
        rtn = (char*) malloc(alen + 1);
        memcpy(rtn, ba, alen);
        rtn[alen] = 0;
    }
    env->ReleaseByteArrayElements(barr, ba, 0);
    return rtn;
}

  
JNIEXPORT jint JNICALL Java_net_meoi_udtrecv_MainActivity_RecvFileFromServer(
		JNIEnv* env, jclass cls,jstring UDTServerAddress,jstring UDTServerPortStr,jstring RemoteFileName,jstring LocalFileName) {
	LOGD("start");
	int argc = 5;
	const char* argv[5];

	argv[1] = jstringToChar(env,UDTServerAddress);
	argv[2] = jstringToChar(env,UDTServerPortStr);
	argv[3] = jstringToChar(env,RemoteFileName);
	argv[4] = jstringToChar(env,LocalFileName);
	/*argv[1] = "192.168.0.100";
	argv[2] = "9000";
	argv[3] = "file.txt";
	argv[4] = "/storage/emulated/0/test.txt";*/
	if ((argc != 5) || (0 == atoi(argv[2]))) {
		cout
				<< "usage: recvfile server_ip server_port remote_filename local_filename"
				<< endl;
		return -1;
	}

	// use this function to initialize the UDT library
	UDT::startup();

	struct addrinfo hints, *peer;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_flags = AI_PASSIVE;
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	UDTSOCKET fhandle = UDT::socket(hints.ai_family, hints.ai_socktype,
			hints.ai_protocol);
	LOGD("##########");
	if (0 != getaddrinfo(argv[1], argv[2], &hints, &peer)) {
		//cout << "incorrect server/peer address. " << argv[1] << ":" << argv[2] << endl;

		return -2;
	}

	// connect to the server, implict bind
	if (UDT::ERROR ==UDT::connect(fhandle, peer->ai_addr, peer->ai_addrlen)) {
		//cout << "connect: " << UDT::getlasterror().getErrorMessage() << endl;
		LOGD("##########connect:%s",UDT::getlasterror().getErrorMessage());
		return -3;
	}

	freeaddrinfo(peer);

	// send name information of the requested file
	int len = strlen(argv[3]);

	if (UDT::ERROR == UDT::send(fhandle, (char*) &len, sizeof(int), 0)) {
		//cout << "send: " << UDT::getlasterror().getErrorMessage() << endl;
		LOGD("##########send:%s",UDT::getlasterror().getErrorMessage());
		return -4;
	}

	if (UDT::ERROR == UDT::send(fhandle, argv[3], len, 0)) {
		//cout << "send: " << UDT::getlasterror().getErrorMessage() << endl;
		LOGD("##########send:%s",UDT::getlasterror().getErrorMessage());
		return -5;
	}

	// get size information
	int64_t size;

	if (UDT::ERROR == UDT::recv(fhandle, (char*) &size, sizeof(int64_t), 0)) {
		//cout << "send: " << UDT::getlasterror().getErrorMessage() << endl;
		LOGD("##########send:%s",UDT::getlasterror().getErrorMessage());
		return -6;
	}

	if (size < 0) {
		//cout << "no such file " << argv[3] << " on the server\n";
		LOGD("##########no such file%s",UDT::getlasterror().getErrorMessage());
		return -7;
	}

	// receive the file
	fstream ofs(argv[4], ios::out | ios::binary | ios::trunc);
	int64_t recvsize;
	int64_t offset = 0;

	if (UDT::ERROR == (recvsize = UDT::recvfile(fhandle, ofs, offset, size))) {
		//cout << "recvfile: " << UDT::getlasterror().getErrorMessage() << endl;
		LOGD("##########recvfile:%s",UDT::getlasterror().getErrorMessage());
		return -8;
	}

	UDT::close(fhandle);

	ofs.close();

	// use this function to release the UDT library
	UDT::cleanup();

	return 0;
}