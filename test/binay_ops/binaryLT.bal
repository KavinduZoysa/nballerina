// RUN: JAVA_HOME=%java_path %testRunScript %s %nballerinacc | filecheck %s

int _bal_result = 0;
public function main() {
    int a = 42;
    int b = 22;
    if (a < b) {
        _bal_result = a;
    }
    else {
        _bal_result = b;
    }
}
// CHECK: RETVAL=22
