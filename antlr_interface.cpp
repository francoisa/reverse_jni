#include <cctype>
#include <future>
#include <thread>
#include <sstream>
#include <iostream>
#include <vector>
#include <map>
#include "antlr_interface.h"

using namespace std;

JavaVM* jvm = NULL;
JavaVMInitArgs vm_args;

#define PATH_SEPARATOR ';'
#define USER_CLASSPATH "build/classes"

map<string, string> inOutMap;
map<thread::id, map<string, string>> tidInOutMap;

antlr::antlr() {
  JNIEnv* jni_env = NULL;
  JavaVMOption* options = new JavaVMOption[4];
  char* classpath = getenv("CLASSPATH");

  if (classpath == nullptr) {
    cerr << "CLASSPATH environment variable is  not set." << endl;
    exit(-1);
  }

  char* ld_library_path = getenv("LD_LIBRARY_PATH");

  if (ld_library_path == nullptr) {
    cerr << "LD_LIBRARY_PATH environment variable is  not set." << endl;
    exit(-1);
  }

  ostringstream cp_oss;
  cp_oss << "-Djava.class.path=" << classpath;
  size_t len = cp_oss.str().size();
  unique_ptr<char> clspath(new char[len+1]);
  cp_oss.str().copy(clspath.get(), len);
  clspath.get()[len] = '\0';
  ostringstream ld_oss;
  ld_oss << "-Djava.library.path=" << ld_library_path;
  len = ld_oss.str().size();
  unique_ptr<char> ldpath(new char[len+1]);
  ld_oss.str().copy(ldpath.get(), len);
  ldpath.get()[len] = '\0';

  options[0].optionString = const_cast<char*>(clspath.get());
  options[1].optionString = const_cast<char*>(ldpath.get());
  options[2].optionString = const_cast<char*>("-Djava.compiler=NONE");
  options[3].optionString = const_cast<char*>("-verbose:class,gc,jni");
  vm_args.version = JNI_VERSION_1_6;
  vm_args.options = options;
  vm_args.nOptions = 4;
  vm_args.ignoreUnrecognized = JNI_FALSE;

  // Create the VM
  jint res = 0;
  exception ex;
  if (jvm == NULL) {
    res = JNI_CreateJavaVM(&jvm, (void**) &jni_env, &vm_args);

    if (res < 0) {
      cerr << "Can't create Java VM: " << res << endl;
      throw ex;
    }
  }

  // Pass NULL as the 3rd argument
  int current = jvm->AttachCurrentThread((void**)&jni_env, NULL);
  if (current < 0) {
    cerr << "Attach failed: " << current << endl;
    throw ex;
  }
  unique_ptr<JNIEnv, void(*)(JNIEnv*)> env(jni_env, [](JNIEnv* jni_env) {
      if (jni_env->ExceptionOccurred()) {
	jni_env->ExceptionDescribe();
      }
      jvm->DetachCurrentThread();
    });

  CalcCls = env->FindClass("net/jmf/calc/Calc");
  if (CalcCls == NULL) {
    cerr << "Parse class is null." << endl;
    throw ex;
  }
  parseId = env->GetStaticMethodID(CalcCls, 
				   "parse", 
				   "(Ljava/lang/String;)Ljava/lang/String;");

  if (parseId == NULL) {
    cerr << "parseId is null." << endl;
    throw ex;
  }
  StringCls = env->FindClass("java/lang/String");
  if (StringCls == NULL) {
    cerr << "String class is null." << endl;
    throw ex;
  }
}

void antlr::async_parse(const string expr) const {
  JNIEnv* jni_env;
  exception ex;

  // Pass NULL as the 3rd argument
  jint res = jvm->AttachCurrentThread((void**)&jni_env, NULL);
  if (res < 0) {
    cerr << "Attach failed: " << res << endl;
    throw ex;
  }

  unique_ptr<JNIEnv, void(*)(JNIEnv*)> env(jni_env, [](JNIEnv* jni_env) {
      if (jni_env->ExceptionOccurred()) {
	jni_env->ExceptionDescribe();
      }
      jvm->DetachCurrentThread();
    });

  jstring jexpr = jni_env->NewStringUTF(expr.c_str());
  if (jexpr == NULL) {
    cerr << "Could not create jexpr string." << cerr;
  }

  jstring jresult = (jstring) jni_env->CallStaticObjectMethod(CalcCls, parseId, jexpr);

  const char *str = jni_env->GetStringUTFChars(jresult, 0);
  // release this string when done with it in order to avoid a memory leak
  string result;
  result.assign(str);
  inOutMap[expr] = result;
  jni_env->ReleaseStringUTFChars(jresult, str);
}

bool antlr::parse(const string& expr, string& result) const {
  auto _future = async(&antlr::async_parse, this, expr);
  _future.get();
  const auto& out = inOutMap.find(expr);
  if (out == inOutMap.end()) {
    return false;
  }
  else {
    result = out->second;
    return !result.empty();
  }
}
