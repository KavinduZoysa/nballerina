// RUN: JAVA_HOME=%java_path %testRunScript %s %nballerinacc | filecheck %s

public function print_str(string val) = external;

public function printu32(int val) = external;

public function main() {
    int|string u = 21;
    int i = <int>u;
    print_str("RESULT=");
    printu32(i);
}
// CHECK: RESULT=21
