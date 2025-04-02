## opencv4当前使用版本

4.11.0

## 3rdparty/protobuf 无法在 Microdroid 内环境使用的问题

故障报错: A/libc: Fatal signal 11 (SIGSEGV), code 128 (SI_KERNEL), fault addr 0x0 in tid xxx

其他十分相似的故障: https://blog.csdn.net/qq_43967413/article/details/132095196

排查后问题即出在 opencv/3rdparty/protobuf 库上，故进行以下步骤与其解耦，并关联上系统protobuf库。

### Step1.重新生成 modules/dnn/misc/onnx/ 下绑定文件

1. 查看 external/protobuf/Android.bp 中检查当前系统中 protobuf 版本, 为 3.9.1
2. 从 https://repo1.maven.org/maven2/com/google/protobuf/protoc/3.9.1/protoc-3.9.1-linux-aarch_64.exe 下载 protoc-3.9.1 工具
3. 针对 modules/dnn/src/onnx/opencv-onnx.proto 文件重新生成c++绑定文件

    ```
    ./protoc-3.9.1-linux-aarch_64.exe opencv-onnx.proto --cpp_out=<OUT_DIR>
    ```

### Step2.增加 external/protobuf 的编译目标

因 microdroid 中的 opencv-payload 全程使用 rtti 编译, 故该 protobuf 依赖库也需。

在 external/protobuf/Android.bp 中增加编译目标：

```diff
external/protobuf$ git diff Android.bp 
diff --git a/Android.bp b/Android.bp
index d852d62..96fd3bc 100644
--- a/Android.bp
+++ b/Android.bp
@@ -274,6 +274,36 @@ cc_library {
     min_sdk_version: "29",
 }
 
+// C++ full library for the platform and host
+// =======================================================
+cc_library {
+    name: "libopencv-protobuf",
+    defaults: ["libprotobuf-cpp-full-defaults"],
+    host_supported: true,
+    vendor_available: true,
+    product_available: true,
+    // TODO(b/153609531): remove when no longer needed.
+    native_bridge_supported: true,
+    target: {
+        android: {
+            static: {
+                enabled: false,
+            },
+        },
+        windows: {
+            enabled: true,
+        },
+    },
+    apex_available: [
+        "//apex_available:platform",
+        "com.android.bluetooth",
+        "com.android.appsearch",
+        "com.android.virt",
+    ],
+    min_sdk_version: "29",
+    rtti: true,
+}
+
 // Compiler library for the host
 // =======================================================
 cc_library {
```
