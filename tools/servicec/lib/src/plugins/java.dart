// Copyright (c) 2015, the Fletch project authors.  Please see the AUTHORS file
// for details. All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.

library servicec.plugins.java;

import 'dart:core' hide Type;
import 'dart:io';

import 'package:path/path.dart';
import 'package:strings/strings.dart' as strings;

import 'shared.dart';
import '../emitter.dart';
import '../struct_layout.dart';

import 'cc.dart' show CcVisitor;

const HEADER = """
// Copyright (c) 2015, the Fletch project authors. Please see the AUTHORS file
// for details. All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE.md file.

// Generated file. Do not edit.
""";

const HEADER_MK = """
# Copyright (c) 2015, the Fletch project authors. Please see the AUTHORS file
# for details. All rights reserved. Use of this source code is governed by a
# BSD-style license that can be found in the LICENSE.md file.

# Generated file. Do not edit.
""";

const FLETCH_API_JAVA = """
package fletch;

public class FletchApi {
  public static native void Setup();
  public static native void TearDown();
  public static native void RunSnapshot(byte[] snapshot);
  public static native void AddDefaultSharedLibrary(String library);
}
""";

const FLETCH_SERVICE_API_JAVA = """
package fletch;

public class FletchServiceApi {
  public static native void Setup();
  public static native void TearDown();
}
""";

const FLETCH_API_JAVA_IMPL = """
#include <jni.h>

#include "fletch_api.h"

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT void JNICALL Java_fletch_FletchApi_Setup(JNIEnv*, jclass) {
  FletchSetup();
}

JNIEXPORT void JNICALL Java_fletch_FletchApi_TearDown(JNIEnv*, jclass) {
  FletchTearDown();
}

JNIEXPORT void JNICALL Java_fletch_FletchApi_RunSnapshot(JNIEnv* env,
                                                         jclass,
                                                         jbyteArray snapshot) {
  int len = env->GetArrayLength(snapshot);
  unsigned char* copy = new unsigned char[len];
  env->GetByteArrayRegion(snapshot, 0, len, reinterpret_cast<jbyte*>(copy));
  FletchRunSnapshot(copy, len);
  delete copy;
}

JNIEXPORT void JNICALL Java_fletch_FletchApi_AddDefaultSharedLibrary(
    JNIEnv* env, jclass, jstring str) {
  const char* library = env->GetStringUTFChars(str, 0);
  FletchAddDefaultSharedLibrary(library);
  env->ReleaseStringUTFChars(str, library);
}

#ifdef __cplusplus
}
#endif
""";

const FLETCH_SERVICE_API_JAVA_IMPL = """
#include <jni.h>

#include "service_api.h"

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT void JNICALL Java_fletch_FletchServiceApi_Setup(JNIEnv*, jclass) {
  ServiceApiSetup();
}

JNIEXPORT void JNICALL Java_fletch_FletchServiceApi_TearDown(JNIEnv*, jclass) {
  ServiceApiTearDown();
}

#ifdef __cplusplus
}
#endif
""";

const JNI_ATTACH_DETACH = """
static JNIEnv* attachCurrentThreadAndGetEnv(JavaVM* vm) {
  AttachEnvType result = NULL;
  if (vm->AttachCurrentThread(&result, NULL) != JNI_OK) {
    // TODO(ager): Nicer error recovery?
    exit(1);
  }
  return reinterpret_cast<JNIEnv*>(result);
}

static void detachCurrentThread(JavaVM* vm) {
  if (vm->DetachCurrentThread() != JNI_OK) {
    // TODO(ager): Nicer error recovery?
    exit(1);
  }
}""";

void generate(String path, Unit unit, String outputDirectory) {
  _generateFletchApis(outputDirectory);
  _generateServiceJava(path, unit, outputDirectory);
  _generateServiceJni(path, unit, outputDirectory);
  _generateServiceJniMakeFiles(path, unit, outputDirectory);
}

void _generateFletchApis(String outputDirectory) {
  String fletchDirectory = join(outputDirectory, 'java', 'fletch');
  String jniDirectory = join(outputDirectory, 'java', 'jni');

  StringBuffer buffer = new StringBuffer(HEADER);
  buffer.writeln();
  buffer.write(FLETCH_API_JAVA);
  writeToFile(fletchDirectory, 'FletchApi', buffer.toString(),
      extension: 'java');

  buffer = new StringBuffer(HEADER);
  buffer.writeln();
  buffer.write(FLETCH_SERVICE_API_JAVA);
  writeToFile(fletchDirectory, 'FletchServiceApi', buffer.toString(),
      extension: 'java');

  buffer = new StringBuffer(HEADER);
  buffer.writeln();
  buffer.write(FLETCH_API_JAVA_IMPL);
  writeToFile(jniDirectory, 'fletch_api_wrapper', buffer.toString(),
      extension: 'cc');

  buffer = new StringBuffer(HEADER);
  buffer.writeln();
  buffer.write(FLETCH_SERVICE_API_JAVA_IMPL);
  writeToFile(jniDirectory, 'fletch_service_api_wrapper', buffer.toString(),
      extension: 'cc');
}

void _generateServiceJava(String path, Unit unit, String outputDirectory) {
  _JavaVisitor visitor = new _JavaVisitor(path);
  visitor.visit(unit);
  String contents = visitor.buffer.toString();
  String directory = join(outputDirectory, 'java', 'fletch');
  // TODO(ager): We should generate a file per service here.
  if (unit.services.length > 1) {
    print('Java plugin: multiple services in one file is not supported.');
  }
  String serviceName = unit.services.first.name;
  writeToFile(directory, serviceName, contents, extension: 'java');
}

void _generateServiceJni(String path, Unit unit, String outputDirectory) {
  _JniVisitor visitor = new _JniVisitor(path);
  visitor.visit(unit);
  String contents = visitor.buffer.toString();
  String directory = join(outputDirectory, 'java', 'jni');
  // TODO(ager): We should generate a file per service here.
  if (unit.services.length > 1) {
    print('Java plugin: multiple services in one file is not supported.');
  }
  String serviceName = unit.services.first.name;
  String file = '${strings.underscore(serviceName)}_wrapper';
  writeToFile(directory, file, contents, extension: 'cc');
}

class _JavaVisitor extends CodeGenerationVisitor {
  _JavaVisitor(String path) : super(path);

  visitUnit(Unit node) {
    writeln(HEADER);
    writeln('package fletch;');
    node.services.forEach(visit);
  }

  visitService(Service node) {
    writeln();
    writeln('public class ${node.name} {');
    writeln('  public static native void Setup();');
    writeln('  public static native void TearDown();');
    node.methods.forEach(visit);
    writeln('}');
  }

  visitMethod(Method node) {
    if (node.inputKind != InputKind.PRIMITIVES) return;
    if (node.outputKind != OutputKind.PRIMITIVE) return;

    String name = node.name;
    String camelName = strings.camelize(name);
    writeln();
    writeln('  public static abstract class ${camelName}Callback {');
    write('    public abstract void handle(');
    writeType(node.returnType);
    writeln(' result);');
    writeln('  }');

    writeln();
    write('  public static native ');
    writeType(node.returnType);
    write(' ${name}(');
    visitArguments(node.arguments);
    writeln(');');
    write('  public static native void ${name}Async(');
    visitArguments(node.arguments);
    if (!node.arguments.isEmpty) write(', ');
    writeln('${camelName}Callback callback);');
  }

  visitArguments(List<Formal> formals) {
    visitNodes(formals, (first) => first ? '' : ', ');
  }

  visitFormal(Formal node) {
    writeType(node.type);
    write(' ${node.name}');
  }

  void writeType(Type node) {
    Map<String, String> types = const {
      'int16': 'short',
      'int32': 'int',
    };
    String type = types[node.identifier];
    write(type);
  }
}

class _JniVisitor extends CcVisitor {
  int methodId = 1;
  String serviceName;

  _JniVisitor(String path) : super(path);

  visitUnit(Unit node) {
    writeln(HEADER);
    writeln('#include <jni.h>');
    writeln('#include <stdlib.h>');
    writeln();
    writeln('#include "service_api.h"');
    node.services.forEach(visit);
  }

  visitService(Service node) {
    serviceName = node.name;

    writeln();

    writeln('#ifdef __cplusplus');
    writeln('extern "C" {');
    writeln('#endif');

    // TODO(ager): Get rid of this if we can. For some reason
    // the jni.h header that is used by the NDK differs.
    writeln();
    writeln('#ifdef ANDROID');
    writeln('  typedef JNIEnv* AttachEnvType;');
    writeln('#else');
    writeln('  typedef void* AttachEnvType;');
    writeln('#endif');

    writeln();
    writeln('static ServiceId service_id_ = kNoServiceId;');

    writeln();
    write('JNIEXPORT void JNICALL Java_fletch_');
    writeln('${serviceName}_Setup(JNIEnv*, jclass) {');
    writeln('  service_id_ = ServiceApiLookup("$serviceName");');
    writeln('}');

    writeln();
    write('JNIEXPORT void JNICALL Java_fletch_');
    writeln('${serviceName}_TearDown(JNIEnv*, jclass) {');
    writeln('  ServiceApiTerminate(service_id_);');
    writeln('}');

    writeln();
    writeln(JNI_ATTACH_DETACH);

    node.methods.forEach(visit);

    writeln();
    writeln('#ifdef __cplusplus');
    writeln('}');
    writeln('#endif');
  }

  visitMethod(Method node) {
    String name = node.name;
    String id = '_k${name}Id';

    writeln();
    write('static const MethodId $id = ');
    writeln('reinterpret_cast<MethodId>(${methodId++});');

    if (node.inputKind != InputKind.PRIMITIVES) return;  // Not handled yet.
    if (node.outputKind != OutputKind.PRIMITIVE) return;

    writeln();
    write('JNIEXPORT ');
    writeType(node.returnType);
    write(' JNICALL Java_fletch_${serviceName}_${name}(');
    write('JNIEnv*, jclass');
    if (!node.arguments.isEmpty) write(', ');
    visitArguments(node.arguments);
    writeln(') {');
    visitMethodBody(id, node);
    writeln('}');

    String callback = ensureCallback(node.returnType,
        node.inputPrimitiveStructLayout);

    writeln();
    write('JNIEXPORT void JNICALL ');
    write('Java_fletch_${serviceName}_${name}Async(');
    write('JNIEnv* _env, jclass');
    if (!node.arguments.isEmpty) write(', ');
    visitArguments(node.arguments);
    writeln(', jobject _callback) {');
    writeln('  jobject callback = _env->NewGlobalRef(_callback);');
    writeln('  JavaVM* vm;');
    writeln('  _env->GetJavaVM(&vm);');
    visitMethodBody(id,
                    node,
                    extraArguments: [ 'vm' ],
                    callback: callback);
    writeln('}');
  }

  void writeType(Type node) {
    Map<String, String> types = const {
      'int16': 'jshort',
      'int32': 'jint'
    };
    String type = types[node.identifier];
    write(type);
  }

  final Map<String, String> callbacks = {};
  String ensureCallback(Type type,
                        StructLayout layout,
                        {bool cStyle: false}) {
    String key = '${type.identifier}_${layout.size}';
    return callbacks.putIfAbsent(key, () {
      String cast(String type) => CcVisitor.cast(type, cStyle);
      String name = 'Unwrap_$key';
      writeln();
      writeln('static void $name(void* raw) {');
      writeln('  char* buffer = ${cast('char*')}(raw);');
      writeln('  int result = *${cast('int*')}(buffer + 32);');
      int offset = 32 + layout.size;
      write('  jobject callback = *${cast('jobject*')}');
      writeln('(buffer + $offset);');
      write('  JavaVM* vm = *${cast('JavaVM**')}');
      writeln('(buffer + $offset + sizeof(void*));');
      writeln('  JNIEnv* env = attachCurrentThreadAndGetEnv(vm);');
      writeln('  jclass clazz = env->GetObjectClass(callback);');
      write('  jmethodID methodId = env->GetMethodID');
      // TODO(ager): For now the return type is hard-coded to int.
      writeln('(clazz, "handle", "(I)V");');
      writeln('  env->CallVoidMethod(callback, methodId, result);');
      writeln('  env->DeleteGlobalRef(callback);');
      writeln('  detachCurrentThread(vm);');
      writeln('  free(buffer);');
      writeln('}');
      return name;
    });
  }
}

void _generateServiceJniMakeFiles(String path, Unit unit, String outputDirectory) {
  String out = join(outputDirectory, 'java');
  String scriptFile = new File.fromUri(Platform.script).path;
  String scriptDir = dirname(scriptFile);
  String fletchLibraryBuildDir = join(scriptDir,
                                      '..',
                                      '..',
                                      'android_build',
                                      'jni');

  String fletchIncludeDir = join(scriptDir,
                                 '..',
                                 '..',
                                 '..',
                                 'include');

  String modulePath = relative(fletchLibraryBuildDir, from: out);
  String includePath = relative(fletchIncludeDir, from: out);

  StringBuffer buffer = new StringBuffer(HEADER_MK);

  buffer.writeln();
  buffer.writeln('LOCAL_PATH := \$(call my-dir)');

  buffer.writeln();
  buffer.writeln('include \$(CLEAR_VARS)');
  buffer.writeln('LOCAL_MODULE := fletch');
  buffer.writeln('LOCAL_CFLAGS := -DFLETCH32 -DANDROID');
  buffer.writeln('LOCAL_LDLIBS := -llog -ldl -rdynamic');

  buffer.writeln();
  buffer.writeln('LOCAL_SRC_FILES := \\');
  buffer.writeln('\tfletch_api_wrapper.cc \\');
  buffer.writeln('\tfletch_service_api_wrapper.cc \\');

  if (unit.services.length > 1) {
    print('Java plugin: multiple services in one file is not supported.');
  }
  String serviceName = unit.services.first.name;
  String file = '${strings.underscore(serviceName)}_wrapper';

  buffer.writeln('\t${file}.cc');

  buffer.writeln();
  buffer.writeln('LOCAL_C_INCLUDES += \$(LOCAL_PATH)');
  buffer.writeln('LOCAL_C_INCLUDES += ${includePath}');
  buffer.writeln('LOCAL_STATIC_LIBRARIES := fletch-library');

  buffer.writeln();
  buffer.writeln('include \$(BUILD_SHARED_LIBRARY)');

  buffer.writeln();
  buffer.writeln('\$(call import-module, ${modulePath})');

  writeToFile(join(out, 'jni'), 'Android', buffer.toString(),
      extension: 'mk');

  buffer = new StringBuffer(HEADER_MK);
  buffer.writeln('APP_STL := gnustl_static');
  buffer.writeln('APP_ABI := all');
  writeToFile(join(out, 'jni'), 'Application', buffer.toString(),
      extension: 'mk');

}
