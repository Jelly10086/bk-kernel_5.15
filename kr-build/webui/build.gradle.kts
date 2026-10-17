import org.jetbrains.kotlin.gradle.targets.js.webpack.KotlinWebpackConfig
import org.jetbrains.compose.web.tasks.UnpackSkikoWasmRuntimeTask

plugins {
    kotlin("multiplatform") version "2.4.0"
    id("org.jetbrains.compose") version "1.11.1"
    id("org.jetbrains.kotlin.plugin.compose") version "2.4.0"
}

kotlin {
    js {
        outputModuleName = "bkControl"
        browser {
            commonWebpackConfig {
                outputFileName = "bkControl.js"
                mode = KotlinWebpackConfig.Mode.PRODUCTION
            }
        }
        binaries.executable()
    }

    sourceSets {
        commonMain.dependencies {
            implementation(compose.runtime)
            implementation(compose.foundation)
            implementation(compose.ui)
            implementation(compose.components.resources)
            implementation("org.jetbrains.kotlinx:kotlinx-coroutines-core:1.10.2")
            implementation("top.yukonga.miuix.kmp:miuix-ui:0.9.3")
            implementation("top.yukonga.miuix.kmp:miuix-preference:0.9.3")
        }
    }
}

compose.resources {
    publicResClass = true
    packageOfResClass = "org.bkkernel.control.generated.resources"
}

val skikoRuntime by configurations.creating {
    isTransitive = false
}
dependencies {
    add(skikoRuntime.name, "org.jetbrains.skiko:skiko-js-wasm-runtime:0.144.6")
}
tasks.withType<UnpackSkikoWasmRuntimeTask>().configureEach {
    skikoRuntimeFiles = skikoRuntime
}
