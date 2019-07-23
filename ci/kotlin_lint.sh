#!/usr/bin/env bash

version="1.0.0-RC16"
detekt_jar="detekt-cli-$version-all.jar"
formatting_jar="detekt-formatting-$version.jar"

#wget -q "https://github.com/arturbosch/detekt/releases/download/$version/$detekt_jar"
#wget -q "https://repo1.maven.org/maven2/io/gitlab/arturbosch/detekt/detekt-formatting/$version/$formatting_jar"

java -jar ${detekt_jar} --plugins ${formatting_jar} --build-upon-default-config -c .kotlinlint.yml -i examples/kotlin/hello_world
