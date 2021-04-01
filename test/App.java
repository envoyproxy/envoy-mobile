package test;

public class App {
    static {
        System.loadLibrary("native");
    }

    public static void main(String[] args) {
        f(123);
    }

    private static native void f(int x);
}