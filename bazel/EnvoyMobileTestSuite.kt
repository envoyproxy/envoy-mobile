package io.envoyproxy.envoymobile.bazel

import junit.framework.JUnit4TestAdapter
import junit.framework.TestSuite
import org.junit.runner.RunWith
import java.io.File
import java.net.URLClassLoader
import java.util.*
import java.util.zip.ZipFile

/**
 * This is class is taken from https://stackoverflow.com/questions/46365464/how-to-run-all-tests-in-bazel-from-a-single-java-test-rule
 *
 * Translated to Kotlin and slightly modified
 *
 */
@RunWith(org.junit.runners.AllTests::class)
object EnvoyMobileTestSuite {
  private const val CLASS_SUFFIX = ".class"

  @JvmStatic
  fun suite(): TestSuite {
    val suite = TestSuite()
    val classLoader = Thread.currentThread().contextClassLoader as URLClassLoader

    // The first entry on the classpath contains the srcs from java_test
    val classesInJar = findClassesInJar(File(classLoader.urLs[0].path))
    for (clazz in classesInJar) {
      val name = Class.forName(clazz)
      if (name != EnvoyMobileTestSuite::class.java) {
        suite.addTest(JUnit4TestAdapter(name))
      }
    }

    return suite
  }

  private fun findClassesInJar(jarFile: File): Set<String> {
    val classNames = mutableSetOf<String>()

    ZipFile(jarFile).use { zipFile ->
      val entries = zipFile.entries()
      for (entry in entries) {
        val entryName = entry.name

        if (entryName.endsWith(CLASS_SUFFIX)) {
          val classNameEnd = entryName.length - CLASS_SUFFIX.length
          classNames.add(entryName.substring(0, classNameEnd).replace('/', '.'))
        }
      }
    }

    return classNames
  }
}