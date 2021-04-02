package test;

public class App {
    static {
        System.loadLibrary("native");
    }

    // For running as main
    public static void main(String[] args) {
        f(123);
    }

    public static native int f(int x);
}